#include "editor/EventHandler.h"

#ifdef _WIN32
    #include <windows.h>
#endif

#include "editor/Route.h"
#include "editor/TrackFurniture.h"

#include <topology.h>
#include <trajectory.h>

#include "editor/Keyboard.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/IntersectionHandler.h"
#include "editor/KeyBindings.h"
#include "editor/MeasureTool.h"
#include "editor/MouseButton.h"
#include "editor/ObjectSelector.h"
#include "editor/RouteObject.h"
#include "editor/ScreenProjector.h"
#include "editor/SplineTool.h"
#include "editor/TrackProfile.h"
#include "editor/TrajectoryPicker.h"
#include "editor/commands/CommandList.h"
#include "editor/commands/AddObject.h"
#include "editor/commands/DeleteObjects.h"
#include "editor/commands/PasteObjects.h"
#include "editor/commands/SelectObjects.h"
#include "editor/commands/ToggleVisibility.h"
#include "editor/settings/CameraSettings.h"
#include "editor/states/EditorState.h"

#include <Journal.h>
#include <filesystem.h>

#include <vsgImGui/imgui.h>

#include <vsg/app/Camera.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

//------------------------------------------------------------------------------
/// Луч камеры через пиксель (та же математика, что в TrajectoryPicker)
//------------------------------------------------------------------------------
static bool build_world_ray(EditorContext& context, int x, int y,
                            vsg::dvec3& origin, vsg::dvec3& point)
{
    const auto camera = context.camera;

    if (!camera || !camera->projectionMatrix || !camera->viewMatrix)
    {
        return false;
    }

    const auto viewport = camera->getViewport();

    if ((viewport.width <= 0) || (viewport.height <= 0))
    {
        return false;
    }

    vsg::dvec2 ndc(0.0, 0.0);
    ndc.set((static_cast<double>(x) - viewport.x) / viewport.width,
            (static_cast<double>(y) - viewport.y) / viewport.height);

    const auto projectionMatrix = camera->projectionMatrix->transform();
    const auto viewMatrix = camera->viewMatrix->transform();

    const bool reverse_depth = (projectionMatrix(2, 2) > 0.0);

    const vsg::dvec3 ndc_near(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0,
                              reverse_depth ? viewport.maxDepth : viewport.minDepth);
    const vsg::dvec3 ndc_far(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0,
                             reverse_depth ? viewport.minDepth : viewport.maxDepth);

    const auto inv_projectionMatrix = vsg::inverse(projectionMatrix);
    const vsg::dvec3 eye_near = inv_projectionMatrix * ndc_near;
    const vsg::dvec3 eye_far = inv_projectionMatrix * ndc_far;

    const vsg::dmat4 eye_to_world = vsg::inverse(viewMatrix);
    origin = eye_to_world * eye_near;
    point = eye_to_world * eye_far;
    return true;
}

//------------------------------------------------------------------------------
/// Дуговая координата траектории, ближайшая к лучу: сэмплируем ось
/// каждые ~5 м и ищем минимум расстояния точка-луч
//------------------------------------------------------------------------------
static double build_traj_coord_at_ray(const Trajectory& trajectory,
                                      const vsg::dvec3& origin,
                                      const vsg::dvec3& ray_point)
{
    const double length = trajectory.getLength();

    if (length < 1.0e-9)
    {
        return 0.0;
    }

    const vsg::dvec3 ray_dir = ray_point - origin;
    const double ray_length = vsg::length(ray_dir);

    if (ray_length < 1.0e-9)
    {
        return 0.0;
    }

    const vsg::dvec3 dir = ray_dir / ray_length;

    const std::size_t samples = std::max<std::size_t>(2,
        static_cast<std::size_t>(std::ceil(length / 5.0)) + 1);

    double best_coord = 0.0;
    double best_distance = std::numeric_limits<double>::max();

    for (std::size_t i = 0; i < samples; ++i)
    {
        const double coord = length * static_cast<double>(i) /
                             static_cast<double>(samples - 1);

        const auto& pp = trajectory.getPosition(coord, 1).position;
        const vsg::dvec3 p(pp.x, pp.y, pp.z);
        const vsg::dvec3 v = p - origin;

        // Расстояние точки до луча
        const double t = vsg::dot(v, dir);
        const vsg::dvec3 closest = origin + dir * t;
        const double distance = vsg::length(p - closest);

        if (distance < best_distance)
        {
            best_distance = distance;
            best_coord = coord;
        }
    }

    return best_coord;
}

//------------------------------------------------------------------------------
/// Пересечение луча с плоскостью земли z=0
//------------------------------------------------------------------------------
static bool build_ray_ground(const vsg::dvec3& origin, const vsg::dvec3& point,
                             vsg::dvec3& hit)
{
    const vsg::dvec3 dir = point - origin;

    if (std::abs(dir.z) < 1.0e-9)
    {
        return false;
    }

    const double t = -origin.z / dir.z;

    if (t <= 0.0)
    {
        return false;
    }

    hit = origin + dir * t;
    return true;
}

//------------------------------------------------------------------------------
/// Снап к концам траекторий (порог 14 px)
//------------------------------------------------------------------------------
static bool spline_snap_end_exists(EditorContext& context, int x, int y,
                                   vsg::dvec3& snapped)
{
    if (context.topology == nullptr)
    {
        return false;
    }

    bool found = false;
    double best_px = 14.0;

    std::lock_guard<std::mutex> lock(context.topology_mutex);
    const traj_list_t* list = context.topology->getTrajectoriesList();

    for (auto it = list->cbegin(); it != list->cend(); ++it)
    {
        const auto& tracks = it.value()->getTracks();

        if (tracks.empty())
        {
            continue;
        }

        const dvec3 raw_ends[2] = {tracks.front().begin_point,
                                   tracks.back().end_point};

        for (const dvec3& raw : raw_ends)
        {
            const vsg::dvec3 end(raw.x, raw.y, raw.z);
            vsg::dvec2 screen;

            if (editor2::project_world_to_screen(context, end, screen))
            {
                const double dx = screen.x - x;
                const double dy = screen.y - y;
                const double dist = std::sqrt(dx * dx + dy * dy);

                if (dist < best_px)
                {
                    best_px = dist;
                    snapped = end;
                    found = true;
                }
            }
        }
    }

    return found;
}

//------------------------------------------------------------------------------
/// Создать объект выбранной метки в точке (команда AddObject - с undo)
//------------------------------------------------------------------------------
static void place_object_at(EditorContext& context, const vsg::dvec3& world)
{
    const auto ref_it = context.objects_ref.find(context.place_object_label);

    if (ref_it == context.objects_ref.end() || !ref_it->second.paged_lod)
    {
        context.status = "Модель не загружена: " + context.place_object_label;
        return;
    }

    const auto object = RouteObject::create(context, ref_it->second.paged_lod,
        context.place_object_label, world,
        vsg::dvec3(0.0, 0.0, context.place_object_rotation_deg));

    context.commands.push(new AddObject(context, object), true);

    context.status = "Поставлен: " + context.place_object_label;
}

EventHandler::EventHandler(EditorContext& context)
    : context_(context)
{
    keyboard = Keyboard::create();

    update_camera_vectors();
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    keyboard->apply(keyPress);

    context_.shift_pressed =
        (keyPress.keyModifier & vsg::MODKEY_Shift) != 0;

    // Ожидание назначения клавиши (окно «Клавиши»): следующий ввод
    // записывается в таблицу привязок
    if (context_.key_binding_wait_action >= 0)
    {
        capture_key_binding(keyPress);
        return;
    }

    handle_shortcuts(keyPress);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    keyboard->apply(keyRelease);

    context_.shift_pressed =
        (keyRelease.keyModifier & vsg::MODKEY_Shift) != 0;
}

void EventHandler::apply(vsg::FocusInEvent& focusIn)
{
    keyboard->apply(focusIn);
}

void EventHandler::apply(vsg::FocusOutEvent& focusOut)
{
    keyboard->apply(focusOut);
}

void EventHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    switch (buttonPress.button)
    {
        case editor2::MOUSE_BUTTON_LEFT:
        {
            is_lmb_pressed_ = true;

            // Установка объекта кликом (TSRE-style): ЛКМ по земле
            // ставит выбранную в Model browser модель
            if (context_.place_object_mode &&
                !context_.place_object_label.empty() &&
                (context_.state == EditorState::EDIT_ROUTE))
            {
                vsg::dvec3 origin;
                vsg::dvec3 point;

                if (build_world_ray(context_, buttonPress.x, buttonPress.y,
                                    origin, point))
                {
                    vsg::dvec3 ground;

                    // Снап к концу пути, иначе - земля z=0
                    vsg::dvec3 snapped;

                    if (spline_snap_end_exists(context_,
                                               buttonPress.x, buttonPress.y,
                                               snapped))
                    {
                        ground = snapped;
                    }
                    else if (!build_ray_ground(origin, point, ground))
                    {
                        ground = context_.place_cursor_world;
                    }

                    place_object_at(context_, ground);
                }

                return;
            }

            // Параметрическая стройка (ТЗ п.6/61): ЛКМ начинает
            // растягивание вдоль выбранной траектории
            if ((context_.build_mode != EditorContext::BuildMode::None) &&
                (context_.selected_trajectory != nullptr) &&
                (context_.state == EditorState::EDIT_ROUTE))
            {
                vsg::dvec3 origin;
                vsg::dvec3 point;

                if (build_world_ray(context_, buttonPress.x, buttonPress.y,
                                    origin, point))
                {
                    context_.build_drag_active = true;
                    context_.build_begin_m = build_traj_coord_at_ray(
                                *context_.selected_trajectory, origin, point);
                    context_.build_end_m = context_.build_begin_m;

                    context_.route->show_build_preview(
                                context_.selected_trajectory,
                                context_.build_begin_m,
                                context_.build_end_m);
                }

                return;
            }

            break;
        }
        case editor2::MOUSE_BUTTON_MIDDLE:
        {
            is_mmb_pressed_ = true;
            break;
        }
        case editor2::MOUSE_BUTTON_RIGHT:
        {
            is_rmb_pressed_ = true;
            break;
        }
        default:
        {
            break;
        }
    }

    // Интерсекторы и выделение объектов (по образцу старого редактора)
    context_.intersection_handler->apply(buttonPress);

    // В режиме "Пути" ЛКМ выбирает траекторию вместо объекта
    if (context_.trajectory_mode &&
        buttonPress.button == editor2::MOUSE_BUTTON_LEFT &&
        context_.trajectory_picker)
    {
        context_.trajectory_picker->apply(buttonPress);
        return;
    }

    if (buttonPress.button == editor2::MOUSE_BUTTON_LEFT &&
        context_.state == EditorState::EDIT_ROUTE)
    {
        // Инструмент измерения (M): ЛКМ ставит точку по сцене
        if (context_.measure_tool && context_.measure_tool->is_active())
        {
            context_.measure_tool->handle_press(buttonPress);
            return;
        }

        // Построение нового пути (N): ЛКМ ставит опорную точку
        // сплайна Catmull-Rom
        if (context_.spline_tool && context_.spline_tool->is_active())
        {
            context_.spline_tool->handle_press(buttonPress);
            return;
        }

        // Гизмо (G): попытка захвата оси/кольца/оси масштаба
        if (context_.gizmo && context_.gizmo->handle_press(buttonPress))
        {
            return;
        }
    }

    // Рамка выделения (ЛКМ + движение); с Ctrl - добавление к выделению
    const bool ctrl_add =
        (buttonPress.mask & vsg::MODKEY_Control) != 0;

    if (buttonPress.button == editor2::MOUSE_BUTTON_LEFT)
    {
        start_selection_rect(buttonPress);
    }

    // Ctrl+ЛКМ зарезервирован под рамку-добавление: без неё клик
    // выбирает объект как раньше
    if (!ctrl_add)
    {
        context_.object_selector->apply(buttonPress);
    }
}

void EventHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

    switch (buttonRelease.button)
    {
        case editor2::MOUSE_BUTTON_LEFT:
        {
            is_lmb_pressed_ = false;

            // Завершение растягивания: строим объект в диапазоне
            if (context_.build_drag_active)
            {
                context_.build_drag_active = false;
                context_.route->hide_build_preview();

                double begin = context_.build_begin_m;
                double end = context_.build_end_m;

                if (end < begin)
                {
                    std::swap(begin, end);
                }

                const std::string name = context_.selected_trajectory_name;
                bool done = false;

                switch (context_.build_mode)
                {
                    case EditorContext::BuildMode::Catenary:
                    {
                        done = TrackFurniture::generate_catenary_poles(
                                    context_, name, context_.build_step_m,
                                    begin, end);
                        break;
                    }

                    case EditorContext::BuildMode::Platform:
                    {
                        done = TrackFurniture::generate_platform(
                                    context_, name, end - begin,
                                    context_.build_platform_width_m,
                                    context_.build_platform_height_m,
                                    context_.build_right_side, begin);
                        break;
                    }

                    case EditorContext::BuildMode::Embankment:
                    {
                        done = TrackFurniture::generate_embankment(
                                    context_, name, begin, end,
                                    context_.build_embank_height_m,
                                    context_.build_embank_shoulder_m);
                        break;
                    }

                    case EditorContext::BuildMode::Cutting:
                    {
                        done = TrackFurniture::generate_cutting(
                                    context_, name, begin, end,
                                    context_.build_cut_depth_m,
                                    context_.build_cut_width_m);
                        break;
                    }

                    case EditorContext::BuildMode::Ditch:
                    {
                        done = TrackFurniture::generate_ditch(
                                    context_, name, begin, end,
                                    context_.build_right_side,
                                    context_.build_ditch_width_m,
                                    context_.build_ditch_depth_m);
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }

                if (done)
                {
                    context_.status = "Построено: " + name +
                            " [" + std::to_string(begin).substr(0, 6) +
                            " .. " + std::to_string(end).substr(0, 6) + " м]";
                }
            }

            break;
        }
        case editor2::MOUSE_BUTTON_MIDDLE:
        {
            is_mmb_pressed_ = false;
            break;
        }
        case editor2::MOUSE_BUTTON_RIGHT:
        {
            is_rmb_pressed_ = false;
            break;
        }
        default:
        {
            break;
        }
    }

    // Завершение перетаскивания гизмо (запись undo-команды)
    if (context_.gizmo)
    {
        context_.gizmo->handle_release(buttonRelease);
    }

    // Завершение рамки выделения
    if (buttonRelease.button == editor2::MOUSE_BUTTON_LEFT)
    {
        finish_selection_rect();
    }

    context_.intersection_handler->apply(buttonRelease);
}

void EventHandler::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled)
    {
        return;
    }

    const vsg::ivec2 new_pos = {moveEvent.x, moveEvent.y};

    if (has_prev_mouse_pos_)
    {
        delta_mouse_pos_ = new_pos - mouse_pos_;
    }
    else
    {
        delta_mouse_pos_ = {0, 0};
    }

    mouse_pos_ = new_pos;
    has_prev_mouse_pos_ = true;

    // Живое превью прокладки пути (SplineTool): позиция курсора
    if (context_.spline_tool && context_.spline_tool->is_active())
    {
        context_.spline_tool->handle_move(moveEvent);
    }

    // Курсор режима установки объектов: точка на земле для маркера
    if (context_.place_object_mode)
    {
        vsg::dvec3 origin;
        vsg::dvec3 point;

        if (build_world_ray(context_, moveEvent.x, moveEvent.y,
                            origin, point))
        {
            vsg::dvec3 ground;
            vsg::dvec3 snapped;

            if (spline_snap_end_exists(context_, moveEvent.x, moveEvent.y,
                                       snapped))
            {
                context_.place_cursor_world = snapped;
                context_.place_cursor_valid = true;
            }
            else if (build_ray_ground(origin, point, ground))
            {
                context_.place_cursor_world = ground;
                context_.place_cursor_valid = true;
            }
        }
    }

    // Живое превью стройки при растягивании (ТЗ п.6/55)
    if (context_.build_drag_active &&
        (context_.selected_trajectory != nullptr) &&
        (context_.build_mode != EditorContext::BuildMode::None))
    {
        vsg::dvec3 origin;
        vsg::dvec3 point;

        if (build_world_ray(context_, moveEvent.x, moveEvent.y,
                            origin, point))
        {
            context_.build_end_m = build_traj_coord_at_ray(
                        *context_.selected_trajectory, origin, point);

            context_.route->show_build_preview(
                        context_.selected_trajectory,
                        context_.build_begin_m,
                        context_.build_end_m);
        }
    }

    // Перетаскивание объектов гизмо
    if (context_.gizmo)
    {
        context_.gizmo->handle_move(moveEvent);
    }

    // Рамка выделения тянется за курсором
    if (context_.selection_rect_active)
    {
        context_.selection_rect_curr = new_pos;
    }

    if (!is_rmb_pressed_)
    {
        return;
    }

    // Обзор камеры при зажатой ПКМ (по образцу CameraHandler)
    const double rotate_speed = context_.camera_settings.rotate_speed;

    yaw_deg_ += static_cast<double>(delta_mouse_pos_.x) * rotate_speed;
    pitch_deg_ -= static_cast<double>(delta_mouse_pos_.y) * rotate_speed;
    pitch_deg_ = std::clamp(pitch_deg_, -89.0, 89.0);

    update_camera_vectors();
}

void EventHandler::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled)
    {
        return;
    }

    // Ctrl+колесо — скорость камеры, колесо — зум (fovy)
    if (keyboard->pressed(vsg::KEY_Control_L, false) ||
        keyboard->pressed(vsg::KEY_Control_R, false))
    {
        double& move_speed = context_.camera_settings.move_speed;

        const double factor = (scrollWheel.delta.y > 0.0) ? 1.1 : (1.0 / 1.1);
        move_speed = std::clamp(move_speed * factor, 0.1, 10000.0);

        return;
    }

    const double zoom_power = context_.camera_settings.zoom_power;

    const auto perspective = context_.camera->get_perspective();
    double& fovy = perspective->fieldOfViewY;
    fovy -= scrollWheel.delta.y * zoom_power;
    fovy = std::clamp(fovy, context_.camera_settings.min_fovy_degrees,
        context_.camera_settings.max_fovy_degrees);
}

void EventHandler::apply(vsg::FrameEvent&)
{
#ifdef _WIN32
    // VSG переводит сканкоды в VK через активную раскладку: в русской
    // раскладке буквы приходят другими кодами. Держим английскую
    // раскладку потока (как во viewer)
    static HKL en_layout = ::LoadKeyboardLayoutA("00000409", KLF_ACTIVATE);

    if ((en_layout != nullptr) && (::GetKeyboardLayout(0) != en_layout))
    {
        ::ActivateKeyboardLayout(en_layout, KLF_SETFORPROCESS);
    }
#endif

    // Гизмо: обновление позиции/масштаба/видимости каждый кадр
    if (context_.gizmo)
    {
        context_.gizmo->update();
    }

    if (!is_rmb_pressed_)
    {
        return;
    }

    move_camera();
}

const vsg::dvec3& EventHandler::get_front() const
{
    return front_;
}

const vsg::dvec3& EventHandler::get_right() const
{
    return right_;
}

const vsg::dvec3& EventHandler::get_up() const
{
    return up_;
}

void EventHandler::handle_shortcuts(vsg::KeyPressEvent& keyPress)
{
    // Не обрабатываем горячие клавиши, пока пользователь вводит текст
    // в полях ImGui
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui::GetIO().WantTextInput)
    {
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        return;
    }

    // P и Esc - режим «Пути» (это режим, а не команда: хардкод)
    switch (keyPress.keyBase)
    {
        case vsg::KEY_Comma:
        {
            if (context_.place_object_mode)
            {
                context_.place_object_rotation_deg -= 15.0;
                context_.status = "Поворот: " +
                    std::to_string(static_cast<int>(context_.place_object_rotation_deg)) + " град";
                return;
            }
            break;
        }
        case vsg::KEY_Period:
        {
            if (context_.place_object_mode)
            {
                context_.place_object_rotation_deg += 15.0;
                context_.status = "Поворот: " +
                    std::to_string(static_cast<int>(context_.place_object_rotation_deg)) + " град";
                return;
            }
            break;
        }
        case vsg::KEY_p:
        case vsg::KEY_P:
        {
            // Режим выбора траекторий "Пути" (этап 2):
            // ЛКМ выбирает ближайшую траекторию
            context_.trajectory_mode = !context_.trajectory_mode;

            context_.status = context_.trajectory_mode
                ? "Path mode: ON (Esc - exit)"
                : "Path mode: OFF";

            return;
        }
        case vsg::KEY_Escape:
        {
            // Выход из режима установки объектов кликом
            if (context_.place_object_mode)
            {
                context_.place_object_mode = false;
                context_.place_object_label.clear();
                context_.status = "Установка объектов: выход";
                return;
            }

            // Выход из режима построения нового пути (клавиша N)
            if (context_.spline_tool && context_.spline_tool->is_active())
            {
                context_.spline_tool->set_active(false);
                context_.status = "Новый путь: выход";
                return;
            }

            // Выход из режима "Пути" (выделение траектории сохраняется)
            if (context_.trajectory_mode)
            {
                context_.trajectory_mode = false;
                context_.status = "Path mode: OFF";
            }

            return;
        }
        default:
        {
            break;
        }
    }

    // Команды - по таблице переназначаемых клавиш (окно «Клавиши»)
    const KeyBindings& keys = context_.key_bindings;

    // Undo с игнорированием Shift: Ctrl+Shift+Z = redo (как раньше)
    if (keys.matches(ACTION_UNDO, keyPress, true))
    {
        if (keyPress.keyModifier & vsg::MODKEY_Shift)
        {
            context_.commands.redo();
            context_.status = "Redo";
        }
        else
        {
            context_.commands.undo();
            context_.status = "Undo";
        }

        return;
    }

    if (keys.matches(ACTION_REDO, keyPress))
    {
        context_.commands.redo();
        context_.status = "Redo";
        return;
    }

    if (keys.matches(ACTION_SAVE_ROUTE, keyPress))
    {
        save_route();
        return;
    }

    if (keys.matches(ACTION_COPY_OBJECTS, keyPress))
    {
        // Копирование выделенных в буфер обмена (вставка - Ctrl+V)
        context_.clipboard_objects = context_.selected_objects;

        if (!context_.clipboard_objects.empty())
        {
            context_.status = "Copied " +
                std::to_string(context_.clipboard_objects.size()) +
                " objects";
        }

        return;
    }

    if (keys.matches(ACTION_PASTE_OBJECTS, keyPress))
    {
        // Вставка копий со смещением (промт п.28-29)
        if (!context_.clipboard_objects.empty())
        {
            context_.commands.push(new PasteObjects(context_), true);
            context_.status = "Pasted objects";
        }

        return;
    }

    if (keys.matches(ACTION_DELETE_OBJECTS, keyPress))
    {
        if (!context_.selected_objects.empty())
        {
            context_.commands.push(new DeleteObjects(context_), true);
            context_.status = "Objects deleted";
        }

        return;
    }

    if (keys.matches(ACTION_FOCUS_ON_SELECTION, keyPress))
    {
        // Фокус на выделении: камера перелетает к габариту объектов
        // (промт п.47: F - focus on selection)
        focus_on_selection();
        return;
    }

    if (keys.matches(ACTION_HIDE_OBJECTS, keyPress))
    {
        // Скрыть/показать выделенные объекты (промт п.8)
        if (!context_.selected_objects.empty())
        {
            context_.commands.push(new ToggleVisibility(context_), true);
            context_.status = "Toggled visibility";
        }

        return;
    }

    if (keys.matches(ACTION_GIZMO_TRANSLATE, keyPress))
    {
        // Гизмо: цикл translate -> rotate -> scale -> off
        cycle_gizmo();
        return;
    }

    if (keys.matches(ACTION_GIZMO_ROTATE, keyPress))
    {
        set_gizmo_mode(GizmoMode::ROTATE);
        return;
    }

    if (keys.matches(ACTION_GIZMO_SCALE, keyPress))
    {
        set_gizmo_mode(GizmoMode::SCALE);
        return;
    }

    // Клавиши инструментов: X - привязка к сетке, [ и ] - шаг сетки,
    // M - инструмент измерения (это режимы, а не команды таблицы)
    if (keyPress.keyModifier & (vsg::MODKEY_Control | vsg::MODKEY_Alt))
    {
        return;
    }

    static const double snap_steps[] = {1.0, 5.0, 10.0};

    const auto snap_step_index = [&snap_steps](double step) -> std::size_t
    {
        for (std::size_t i = 0; i < 3; ++i)
        {
            if (snap_steps[i] == step)
            {
                return i;
            }
        }

        return 0;
    };

    switch (keyPress.keyBase)
    {
        case vsg::KEY_x:
        case vsg::KEY_X:
        {
            // Привязка перемещения к сетке (промт п.28)
            context_.snap_enabled = !context_.snap_enabled;

            context_.status = context_.snap_enabled
                ? "Snap: ON, шаг " +
                    std::to_string(static_cast<int>(context_.snap_step)) +
                    " м ([ и ] - шаг)"
                : "Snap: OFF";

            return;
        }
        case vsg::KEY_Leftbracket:
        {
            // Уменьшение шага сетки: 10 -> 5 -> 1
            std::size_t index = snap_step_index(context_.snap_step);

            if (index > 0)
            {
                --index;
            }

            context_.snap_step = snap_steps[index];
            context_.status = "Snap шаг: " +
                std::to_string(static_cast<int>(context_.snap_step)) + " м";

            return;
        }
        case vsg::KEY_Rightbracket:
        {
            // Увеличение шага сетки: 1 -> 5 -> 10
            std::size_t index = snap_step_index(context_.snap_step);

            if (index < 2)
            {
                ++index;
            }

            context_.snap_step = snap_steps[index];
            context_.status = "Snap шаг: " +
                std::to_string(static_cast<int>(context_.snap_step)) + " м";

            return;
        }
        case vsg::KEY_m:
        case vsg::KEY_M:
        {
            // Инструмент измерения (промт: расстояния и дельты Z)
            if (context_.measure_tool)
            {
                const bool active = !context_.measure_tool->is_active();
                context_.measure_tool->set_active(active);

                context_.status = active
                    ? "Измерение: клики ставят точки (M - выход)"
                    : "Измерение выключено";
            }

            return;
        }
        case vsg::KEY_n:
        case vsg::KEY_N:
        {
            // Построение нового пути кликами (промт: spline-инструмент):
            // ЛКМ ставит опорные точки, окно «Новый путь» создаёт путь
            if (context_.spline_tool)
            {
                const bool active = !context_.spline_tool->is_active();
                context_.spline_tool->set_active(active);

                context_.status = active
                    ? "Новый путь: клики ставят точки (Backspace - удалить, N - выход)"
                    : "Новый путь: выход";
            }

            return;
        }
        case vsg::KEY_BackSpace:
        {
            // Удаление последней опорной точки нового пути
            if (context_.spline_tool && context_.spline_tool->is_active())
            {
                context_.spline_tool->remove_last();
            }

            return;
        }
        default:
        {
            return;
        }
    }
}

void EventHandler::capture_key_binding(vsg::KeyPressEvent& keyPress)
{
    const int action_index = context_.key_binding_wait_action;

    if (action_index < 0 || action_index >= TOTAL_ACTIONS)
    {
        context_.key_binding_wait_action = -1;
        return;
    }

    const Action action = static_cast<Action>(action_index);

    // Esc отменяет назначение
    if (keyPress.keyBase == vsg::KEY_Escape)
    {
        context_.key_binding_wait_action = -1;
        context_.status = "Назначение клавиши отменено";
        return;
    }

    // Отдельные нажатия модификаторов пропускаем - ждём клавишу
    switch (keyPress.keyBase)
    {
        case vsg::KEY_Control_L:
        case vsg::KEY_Control_R:
        case vsg::KEY_Shift_L:
        case vsg::KEY_Shift_R:
        case vsg::KEY_Alt_L:
        case vsg::KEY_Alt_R:
        {
            return;
        }
        default:
        {
            break;
        }
    }

    vsg::KeySymbol key = keyPress.keyBase;

    if (key == static_cast<vsg::KeySymbol>(0))
    {
        key = keyPress.keyBase;
    }

    const std::uint16_t modifiers = static_cast<std::uint16_t>(
        keyPress.keyModifier & (vsg::MODKEY_Control | vsg::MODKEY_Shift |
            vsg::MODKEY_Alt));

    context_.key_bindings.assign(action, key, modifiers);

    // Сохраняем в конфиг настроек редактора (секция Keys)
    const FileSystem& fs = FileSystem::getInstance();
    const std::string cfg_path = fs.combinePath(fs.getConfigDir(),
        "editor-settings.xml");

    if (context_.key_bindings.save(cfg_path))
    {
        context_.status = std::string("Клавиша \"") +
            to_c_string(action) + "\" = " +
            context_.key_bindings.to_string(action);
    }
    else
    {
        context_.status = "Не удалось сохранить клавиши в " + cfg_path;
    }

    context_.key_binding_wait_action = -1;
}

void EventHandler::start_selection_rect(
    const vsg::ButtonPressEvent& buttonPress)
{
    // Клик по окну ImGui не начинает рамку
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        return;
    }

    context_.selection_rect_active = true;
    context_.selection_rect_start = {buttonPress.x, buttonPress.y};
    context_.selection_rect_curr = context_.selection_rect_start;

    rect_ctrl_add_ = (buttonPress.mask & vsg::MODKEY_Control) != 0;
}

void EventHandler::finish_selection_rect()
{
    if (!context_.selection_rect_active)
    {
        return;
    }

    context_.selection_rect_active = false;

    // Рамка засчитывается только если курсор реально двигался,
    // иначе это был обычный клик выбора
    const vsg::ivec2 size = context_.selection_rect_curr -
        context_.selection_rect_start;

    if (std::abs(size.x) < 5 && std::abs(size.y) < 5)
    {
        return;
    }

    select_objects_in_rect(rect_ctrl_add_);
}

void EventHandler::select_objects_in_rect(bool ctrl_add)
{
    const double min_x = std::min(context_.selection_rect_start.x,
        context_.selection_rect_curr.x);
    const double max_x = std::max(context_.selection_rect_start.x,
        context_.selection_rect_curr.x);
    const double min_y = std::min(context_.selection_rect_start.y,
        context_.selection_rect_curr.y);
    const double max_y = std::max(context_.selection_rect_start.y,
        context_.selection_rect_curr.y);

    // Ищем объекты, чей центр габарита попал в рамку
    RouteObjects found_objects;

    {
        std::lock_guard<std::mutex> lock_guard(
            context_.static_objects_mutex);

        for (const auto& object : context_.static_objects)
        {
            // Позиция: центр габарита (или translation, если габарит
            // ещё не посчитан)
            vsg::dvec3 center = object->get_translation();

            const vsg::dbox& bounds = object->get_bounds();

            if (bounds.valid())
            {
                center = (bounds.min + bounds.max) * 0.5;
            }

            vsg::dvec2 screen = {0.0, 0.0};

            if (!editor2::project_world_to_screen(context_, center,
                screen))
            {
                continue;
            }

            if (screen.x < min_x || screen.x > max_x ||
                screen.y < min_y || screen.y > max_y)
            {
                continue;
            }

            found_objects.emplace_back(object);
        }
    }

    if (found_objects.empty())
    {
        // Без Ctrl - пустая рамка снимает выделение (как клик мимо)
        if (!ctrl_add && !context_.selected_objects.empty())
        {
            SelectObjects* const select_objects_command =
                new SelectObjects(context_);

            select_objects_command->objects_to_deselect =
                context_.selected_objects;
            select_objects_command->update_description();

            context_.commands.push(select_objects_command, true);
        }

        return;
    }

    SelectObjects* const select_objects_command =
        new SelectObjects(context_);

    if (ctrl_add)
    {
        // Добавление к текущему выделению
        for (const auto& object : found_objects)
        {
            if (!object->get_is_selected())
            {
                select_objects_command->objects_to_select.emplace_back(
                    object);
            }
        }
    }
    else
    {
        // Замена выделения
        select_objects_command->objects_to_select = found_objects;
        select_objects_command->objects_to_deselect =
            context_.selected_objects;
    }

    select_objects_command->update_description();

    context_.commands.push(select_objects_command, true);

    context_.status = "Выделено объектов: " +
        std::to_string(select_objects_command->objects_to_select.size());
}

void EventHandler::cycle_gizmo()
{
    if (!context_.gizmo)
    {
        return;
    }

    const GizmoMode next = Gizmo::next_mode(context_.gizmo->get_mode());

    set_gizmo_mode(next);
}

void EventHandler::set_gizmo_mode(GizmoMode mode)
{
    if (!context_.gizmo)
    {
        return;
    }

    context_.gizmo->set_mode(mode);

    switch (mode)
    {
        case GizmoMode::TRANSLATE:
        {
            context_.status = "Gizmo: перемещение (стрелки)";
            break;
        }
        case GizmoMode::ROTATE:
        {
            context_.status = "Gizmo: поворот (кольца)";
            break;
        }
        case GizmoMode::SCALE:
        {
            context_.status = "Gizmo: масштаб";
            break;
        }
        default:
        {
            context_.status = "Gizmo: выключено";
            break;
        }
    }
}

void EventHandler::focus_on_selection()
{
    if (context_.selected_objects.empty())
    {
        return;
    }

    // Общий габарит выделения
    vsg::dbox bounds;

    bool has_bounds = false;

    for (const auto& object : context_.selected_objects)
    {
        const vsg::dbox& object_bounds = object->get_bounds();

        if (object_bounds.valid())
        {
            if (has_bounds)
            {
                bounds.add(object_bounds.min);
                bounds.add(object_bounds.max);
            }
            else
            {
                bounds = object_bounds;
                has_bounds = true;
            }
        }
    }

    if (!has_bounds)
    {
        return;
    }

    const vsg::dvec3 center =
            (bounds.min + bounds.max) * 0.5;

    double radius = vsg::length(bounds.max - bounds.min) * 0.5;

    if (radius < 5.0)
    {
        radius = 5.0;
    }

    // Камера отлетает по текущему направлению взгляда так, чтобы
    // габарит целиком попал в кадр (дистанция ~3 радиуса)
    const vsg::ref_ptr<Camera> camera = context_.camera;

    const vsg::ref_ptr<vsg::LookAt> look_at = camera->get_look_at();

    if (!camera || !look_at)
    {
        return;
    }

    const double distance = std::max(radius * 3.0, 20.0);

    look_at->center = center;
    look_at->eye =
            center - front_ * distance + vsg::dvec3(0.0, 0.0, radius * 0.4);
    look_at->up = {0.0, 0.0, 1.0};

    context_.status = "Focused on selection";
}

void EventHandler::update_camera_vectors()
{
    const double yaw_rad = vsg::radians(yaw_deg_);
    const double pitch_rad = vsg::radians(pitch_deg_);

    front_ = vsg::normalize(vsg::dvec3(
        sin(yaw_rad) * cos(pitch_rad),
        cos(yaw_rad) * cos(pitch_rad),
        sin(pitch_rad)
    ));

    const vsg::dvec3 world_up = {0.0, 0.0, 1.0};

    right_ = vsg::normalize(vsg::cross(front_, world_up));
    up_ = vsg::normalize(vsg::cross(right_, front_));
}

void EventHandler::move_camera()
{
    const auto look_at = context_.camera->get_look_at();

    const double move_speed = context_.camera_settings.move_speed *
        context_.delta_time;

    const double forward_state =
        static_cast<double>(keyboard->pressed(vsg::KEY_W, false) ||
                            keyboard->pressed(vsg::KEY_w, false)) -
        static_cast<double>(keyboard->pressed(vsg::KEY_S, false) ||
                            keyboard->pressed(vsg::KEY_s, false));

    const double right_state =
        static_cast<double>(keyboard->pressed(vsg::KEY_D, false) ||
                            keyboard->pressed(vsg::KEY_d, false)) -
        static_cast<double>(keyboard->pressed(vsg::KEY_A, false) ||
                            keyboard->pressed(vsg::KEY_a, false));

    // Q/E — вниз/вверх вдоль мировой вертикали
    const double up_state =
        static_cast<double>(keyboard->pressed(vsg::KEY_E, false) ||
                            keyboard->pressed(vsg::KEY_e, false)) -
        static_cast<double>(keyboard->pressed(vsg::KEY_Q, false) ||
                            keyboard->pressed(vsg::KEY_q, false));

    const vsg::dvec3 world_up = {0.0, 0.0, 1.0};

    look_at->eye += front_ * move_speed * forward_state;
    look_at->eye += right_ * move_speed * right_state;
    look_at->eye += world_up * move_speed * up_state;

    look_at->center = look_at->eye + front_;
}

void EventHandler::save_route() const
{
    const FileSystem& fs = FileSystem::getInstance();
    const std::string save_dir = fs.combinePath(
        context_.route_dir, "topology", "map");

    try
    {
        // Резервная копия предыдущего состояния
        std::filesystem::copy_file(
            fs.combinePath(save_dir, "route1.map"),
            fs.combinePath(save_dir, "route1.map.prev"),
            std::filesystem::copy_options::overwrite_existing
        );
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        Journal::instance()->error(e.what());
    }

    // Перезаписываем рабочую копию
    const std::string route_map_path = fs.combinePath(save_dir, "route1.map");

    std::ofstream route_map_file(route_map_path);

    if (!route_map_file.is_open())
    {
        Journal::instance()->error(QString("Failed to open file %1 for writing")
            .arg(route_map_path.c_str()));

        context_.status = "Save failed";

        return;
    }

    {
        std::lock_guard<std::mutex> lock_guard(context_.static_objects_mutex);

        for (const auto& object : context_.static_objects)
        {
            const vsg::dvec3& translation = object->get_translation();
            const vsg::dvec3 rotation_deg = -object->get_rotation_deg();

            route_map_file << object->label << "," <<
                translation.x << "," << translation.y << "," <<
                translation.z << "," <<
                rotation_deg.x << "," << rotation_deg.y << "," <<
                rotation_deg.z << ";\n";
        }
    }

    // Профили участков пути + сгенерированный обвес + слои объектов
    // (track-edit.conf; формат route1.map не затрагивается)
    {
        const std::string track_edit_path = fs.combinePath(
            context_.route_dir, "track-edit.conf");

        if (!save_track_edit_config(track_edit_path, context_))
        {
            context_.status = "Failed to save track-edit.conf";
            return;
        }
    }

    context_.status = "Route saved: " + route_map_path;
}
