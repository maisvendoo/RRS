#include "editor/SplineTool.h"

#include <topology.h>
#include <trajectory.h>


#include "editor/EditorContext.h"
#include "editor/IntersectionHandler.h"
#include "editor/MouseButton.h"
#include "editor/ScreenProjector.h"
#include "editor/TrackFurniture.h"
#include "editor/TrackProfile.h"
#include "editor/states/EditorState.h"

#include <filesystem.h>

#include <vsgImGui/imgui.h>

#include <vsg/nodes/Group.h>
#include <vsg/ui/PointerEvent.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

SplineTool::SplineTool(EditorContext& context)
    : context_(context)
{
}

void SplineTool::set_active(bool active)
{
    active_ = active;
}

bool SplineTool::is_active() const
{
    return active_;
}

//------------------------------------------------------------------------------
/// Луч камеры через пиксель (математика как в EventHandler)
//------------------------------------------------------------------------------
static bool spline_world_ray(EditorContext& context, int x, int y,
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

    vsg::dvec2 ndc((static_cast<double>(x) - viewport.x) / viewport.width,
                   (static_cast<double>(y) - viewport.y) / viewport.height);

    const auto projectionMatrix = camera->projectionMatrix->transform();
    const auto viewMatrix = camera->viewMatrix->transform();
    const bool reverse_depth = (projectionMatrix(2, 2) > 0.0);

    const vsg::dvec3 ndc_near(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0,
                              reverse_depth ? viewport.maxDepth : viewport.minDepth);
    const vsg::dvec3 ndc_far(ndc.x * 2.0 - 1.0, ndc.y * 2.0 - 1.0,
                             reverse_depth ? viewport.minDepth : viewport.maxDepth);

    const auto inv_projection = vsg::inverse(projectionMatrix);
    const vsg::dvec3 eye_near = inv_projection * ndc_near;
    const vsg::dvec3 eye_far = inv_projection * ndc_far;
    const vsg::dmat4 eye_to_world = vsg::inverse(viewMatrix);

    origin = eye_to_world * eye_near;
    point = eye_to_world * eye_far;
    return true;
}

//------------------------------------------------------------------------------
/// Точка на плоскости земли z=0 под лучом
//------------------------------------------------------------------------------
static bool spline_ray_ground(const vsg::dvec3& origin, const vsg::dvec3& point,
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
/// Снап к концам траекторий: экранная близость <= threshold_px
//------------------------------------------------------------------------------
static bool spline_snap_to_end(EditorContext& context, int x, int y,
                               vsg::dvec3& snapped)
{
    if (context.topology == nullptr)
    {
        return false;
    }

    const double threshold_px = 14.0;

    bool found = false;
    double best_px = threshold_px;

    std::lock_guard<std::mutex> lock(context.topology_mutex);
    const traj_list_t* list = context.topology->getTrajectoriesList();

    for (auto it = list->cbegin(); it != list->cend(); ++it)
    {
        Trajectory* traj = it.value();
        const auto& tracks = traj->getTracks();

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
/// Шаблон секции пути (TSRE-style): прямая или дуга
//------------------------------------------------------------------------------
struct SectionTemplate
{
    const char* name;
    bool curve;         // false - прямая
    double length_m;    // длина секции (для дуги - по дуге)
    double radius_m;    // радиус (только дуга)
    double angle_deg;   // угол дуги (только дуга)
};

static const SectionTemplate section_templates[] =
{
    {u8"Свободный сплайн", false, 0.0, 0.0, 0.0},
    {u8"Прямая 25 м", false, 25.0, 0.0, 0.0},
    {u8"Прямая 50 м", false, 50.0, 0.0, 0.0},
    {u8"Прямая 100 м", false, 100.0, 0.0, 0.0},
    {u8"Прямая 200 м", false, 200.0, 0.0, 0.0},
    {u8"Кривая R300 10" "°", true, 0.0, 300.0, 10.0},
    {u8"Кривая R500 10" "°", true, 0.0, 500.0, 10.0},
    {u8"Кривая R800 10" "°", true, 0.0, 800.0, 10.0},
    {u8"Кривая R300 25" "°", true, 0.0, 300.0, 25.0},
    {u8"Кривая R500 25" "°", true, 0.0, 500.0, 25.0},
    {u8"Кривая R800 25" "°", true, 0.0, 800.0, 25.0},
    {u8"Кривая R300 45" "°", true, 0.0, 300.0, 45.0},
    {u8"Кривая R500 45" "°", true, 0.0, 500.0, 45.0},
    {u8"Кривая R800 45" "°", true, 0.0, 800.0, 45.0},
};

constexpr std::size_t SECTION_COUNT =
    sizeof(section_templates) / sizeof(section_templates[0]);

//------------------------------------------------------------------------------
/// Точки секции от start по heading (мировые координаты, z=0):
/// для прямой - конец; для дуги - 4 точки по хорде + конец (и поворот
/// heading на угол дуги)
//------------------------------------------------------------------------------
static std::vector<vsg::dvec3> section_points(const vsg::dvec3& start,
                                              double& heading_deg,
                                              const SectionTemplate& section)
{
    std::vector<vsg::dvec3> result;

    auto dir_vec = [](double deg)
    {
        const double rad = deg * 0.017453292519943295;
        // 0 град = север (+Y), по часовой
        return vsg::dvec3(std::sin(rad), std::cos(rad), 0.0);
    };

    if (!section.curve)
    {
        result.push_back(start + dir_vec(heading_deg) * section.length_m);
        return result;
    }

    // Дуга: шаг 1/4 угла, касательная поворачивается равномерно
    const double total_angle = section.angle_deg;
    const int steps = 4;

    vsg::dvec3 position = start;
    double local_heading = heading_deg;

    for (int i = 0; i < steps; ++i)
    {
        const double step_angle = total_angle / steps;
        const double chord = 2.0 * section.radius_m *
                std::sin(step_angle * 0.017453292519943295 / 2.0);

        local_heading += step_angle * 0.5;
        position += dir_vec(local_heading) * chord;
        local_heading += step_angle * 0.5;

        result.push_back(position);
    }

    heading_deg = local_heading;
    return result;
}

void SplineTool::handle_press(const vsg::ButtonPressEvent& buttonPress)
{
    // Постановка точки - по образцу MeasureTool::handle_press
    if (buttonPress.handled ||
        buttonPress.button != editor2::MOUSE_BUTTON_LEFT)
    {
        return;
    }

    // Клик по окну ImGui не должен ставить точку
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (context_.state != EditorState::EDIT_ROUTE)
    {
        return;
    }

    // Постановка точки по образцу TSRE:
    // 1) снап к концу существующей траектории (продолжение пути);
    // 2) иначе - пересечение луча с плоскостью земли z=0
    //    (сцена из линий лучом не пересекается)
    vsg::dvec3 snapped;

    if (spline_snap_to_end(context_, buttonPress.x, buttonPress.y, snapped))
    {
        points_.push_back(snapped);
        return;
    }

    // Секционная прокладка: секция от последней точки по heading
    // (первая точка - как обычно: клик по земле задаёт начало)
    if ((section_index_ > 0) && (section_index_ <
            static_cast<int>(SECTION_COUNT)) && !points_.empty())
    {
        const SectionTemplate& section =
            section_templates[static_cast<std::size_t>(section_index_)];

        for (const vsg::dvec3& p : section_points(points_.back(),
                                                  heading_deg_, section))
        {
            points_.push_back(p);
        }

        return;
    }

    vsg::dvec3 origin;
    vsg::dvec3 point;

    if (spline_world_ray(context_, buttonPress.x, buttonPress.y,
                         origin, point))
    {
        vsg::dvec3 ground;

        if (spline_ray_ground(origin, point, ground))
        {
            points_.push_back(ground);
        }
    }
}

void SplineTool::handle_move(const vsg::MoveEvent& moveEvent)
{
    if (!active_)
    {
        return;
    }

    vsg::dvec3 origin;
    vsg::dvec3 point;

    if (spline_world_ray(context_, moveEvent.x, moveEvent.y,
                         origin, point))
    {
        vsg::dvec3 snapped;

        if (spline_snap_to_end(context_, moveEvent.x, moveEvent.y, snapped))
        {
            cursor_world_ = snapped;
            has_cursor_ = true;
            return;
        }

        vsg::dvec3 ground;

        if (spline_ray_ground(origin, point, ground))
        {
            cursor_world_ = ground;
            has_cursor_ = true;
        }
    }
}

bool SplineTool::remove_last()
{
    if (points_.empty())
    {
        return false;
    }

    points_.pop_back();

    return true;
}

void SplineTool::clear()
{
    points_.clear();
}

std::vector<vsg::dvec3> SplineTool::catmull_rom_points(
    const std::vector<vsg::dvec3>& control_points,
    std::size_t segments_per_interval)
{
    if (control_points.size() < 2 || segments_per_interval == 0)
    {
        return control_points;
    }

    std::vector<vsg::dvec3> curve;
    curve.reserve((control_points.size() - 1) * segments_per_interval + 1);

    // Равномерный Catmull-Rom: за пределами концов опорные точки
    // дублируются, кривая проходит через все опорные точки
    for (std::size_t i = 0; i + 1 < control_points.size(); ++i)
    {
        const vsg::dvec3& p0 =
            control_points[i > 0 ? i - 1 : i];
        const vsg::dvec3& p1 = control_points[i];
        const vsg::dvec3& p2 = control_points[i + 1];
        const vsg::dvec3& p3 =
            control_points[i + 2 < control_points.size() ? i + 2 : i + 1];

        for (std::size_t s = 0; s < segments_per_interval; ++s)
        {
            const double t = static_cast<double>(s) /
                static_cast<double>(segments_per_interval);

            const double t2 = t * t;
            const double t3 = t2 * t;

            curve.push_back(0.5 * (2.0 * p1 +
                (-p0 + p2) * t +
                (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2 +
                (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3));
        }
    }

    curve.push_back(control_points.back());

    return curve;
}

std::vector<vsg::dvec3> SplineTool::sample_spline_by_step(
    const std::vector<vsg::dvec3>& control_points, double step_m)
{
    // Плотная кривая: 20 сегментов на интервал между опорными точками
    const std::vector<vsg::dvec3> dense =
        catmull_rom_points(control_points, 20);

    if (dense.size() < 2 || step_m < 0.1)
    {
        return dense;
    }

    // Равномерный шаг по длине кривой (конец включается всегда)
    std::vector<vsg::dvec3> samples;
    samples.push_back(dense.front());

    double rest = step_m;

    for (std::size_t i = 1; i < dense.size(); ++i)
    {
        const vsg::dvec3 segment = dense[i] - dense[i - 1];
        const double segment_length = vsg::length(segment);

        if (segment_length < 1.0e-9)
        {
            continue;
        }

        const vsg::dvec3 direction = segment / segment_length;

        double passed = 0.0;

        while (segment_length - passed >= rest)
        {
            passed += rest;
            samples.push_back(dense[i - 1] + direction * passed);
            rest = step_m;
        }

        rest -= segment_length - passed;
    }

    // Конечная точка кривой - всегда последняя
    if (vsg::length(dense.back() - samples.back()) > 1.0e-6)
    {
        samples.push_back(dense.back());
    }

    return samples;
}

void SplineTool::draw_overlay()
{
    if (!active_)
    {
        return;
    }

    ImDrawList* const draw_list = ImGui::GetBackgroundDrawList();

    // Превью секции: от последней точки по heading (шаблон из списка)
    if (!points_.empty() && section_index_ > 0 &&
        section_index_ < static_cast<int>(SECTION_COUNT))
    {
        const SectionTemplate& section =
            section_templates[static_cast<std::size_t>(section_index_)];

        double heading_copy = heading_deg_;
        vsg::dvec2 last_screen;

        if (editor2::project_world_to_screen(context_, points_.back(),
                                             last_screen))
        {
            for (const vsg::dvec3& p : section_points(points_.back(),
                                                     heading_copy, section))
            {
                vsg::dvec2 screen;

                if (editor2::project_world_to_screen(context_, p, screen))
                {
                    draw_list->AddLine(
                                ImVec2(static_cast<float>(last_screen.x),
                                       static_cast<float>(last_screen.y)),
                                ImVec2(static_cast<float>(screen.x),
                                       static_cast<float>(screen.y)),
                                IM_COL32(255, 200, 60, 230), 3.0f);
                    last_screen = screen;
                }
            }
        }
    }

    // Живое превью следующего сегмента: от последней точки к курсору
    // (как предпросмотр секции в TSRE)
    if (!points_.empty() && has_cursor_)
    {
        vsg::dvec2 from_screen;

        if (editor2::project_world_to_screen(context_, points_.back(),
                                             from_screen))
        {
            vsg::dvec2 to_screen;

            if (editor2::project_world_to_screen(context_, cursor_world_,
                                                 to_screen))
            {
                draw_list->AddLine(
                            ImVec2(static_cast<float>(from_screen.x),
                                   static_cast<float>(from_screen.y)),
                            ImVec2(static_cast<float>(to_screen.x),
                                   static_cast<float>(to_screen.y)),
                            IM_COL32(120, 220, 120, 220), 2.0f);
            }
        }
    }

    // Превью кривой Catmull-Rom: экранные проекции пересчитываются
    // каждый кадр (паттерн MeasureTool::draw_overlay)
    if (points_.size() >= 2)
    {
        const std::vector<vsg::dvec3> curve =
            catmull_rom_points(points_, 20);

        bool has_prev_screen = false;
        ImVec2 prev_screen = {0.0f, 0.0f};

        for (const vsg::dvec3& point : curve)
        {
            vsg::dvec2 screen = {0.0, 0.0};

            if (!editor2::project_world_to_screen(context_, point, screen))
            {
                has_prev_screen = false;
                continue;
            }

            const ImVec2 curr_screen(static_cast<float>(screen.x),
                static_cast<float>(screen.y));

            if (has_prev_screen)
            {
                draw_list->AddLine(prev_screen, curr_screen,
                    IM_COL32(80, 200, 255, 220), 2.0f);
            }

            prev_screen = curr_screen;
            has_prev_screen = true;
        }
    }

    // Опорные точки - оранжевые маркеры
    for (const vsg::dvec3& point : points_)
    {
        vsg::dvec2 screen = {0.0, 0.0};

        if (editor2::project_world_to_screen(context_, point, screen))
        {
            draw_list->AddCircleFilled(
                ImVec2(static_cast<float>(screen.x),
                    static_cast<float>(screen.y)),
                5.0f, IM_COL32(255, 140, 0, 255), 12);
        }
    }

    // Окно «Новый путь» (правый нижний угол, по образцу «Измерение»)
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x - 16.0f, io.DisplaySize.y - 16.0f),
        ImGuiCond_Always, ImVec2(1.0f, 1.0f));

    if (!ImGui::Begin("Новый путь", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing))
    {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted(
        "Кликните по сцене, чтобы поставить точку");
    ImGui::TextUnformatted("Backspace - удалить точку, N/Esc - выход");

    // Прокладка секциями (TSRE-style): шаблон + направление
    const char* names[SECTION_COUNT];
    for (std::size_t i = 0; i < SECTION_COUNT; ++i)
    {
        names[i] = section_templates[i].name;
    }

    if (ImGui::Combo(u8"Секция", &section_index_, names,
                     static_cast<int>(SECTION_COUNT)))
    {
        if (points_.empty())
        {
            heading_deg_ = 0.0;
        }
    }

    if (section_index_ > 0)
    {
        float heading_f = static_cast<float>(heading_deg_);
        if (ImGui::SliderAngle(u8"Направление", &heading_f, 0.0f, 360.0f))
        {
            heading_deg_ = heading_f;
        }
        ImGui::TextDisabled(u8"Секция пристыкуется к последней точке "
                              u8"в выбранном направлении");
    }

    // Список опорных точек с координатами и длинами сегментов
    if (ImGui::BeginTable("spline_table", 5,
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("N");
        ImGui::TableSetupColumn("X, м");
        ImGui::TableSetupColumn("Y, м");
        ImGui::TableSetupColumn("Z, м");
        ImGui::TableSetupColumn("Сегмент, м");
        ImGui::TableHeadersRow();

        for (std::size_t i = 0; i < points_.size(); ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%zu", i + 1);

            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", points_[i].x);

            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", points_[i].y);

            ImGui::TableNextColumn();
            ImGui::Text("%10.3f", points_[i].z);

            ImGui::TableNextColumn();

            if (i > 0)
            {
                ImGui::Text("%10.3f",
                    vsg::length(points_[i] - points_[i - 1]));
            }
            else
            {
                ImGui::TextUnformatted("-");
            }
        }

        ImGui::EndTable();

        // Суммарная длина ломаной по опорным точкам
        if (points_.size() >= 2)
        {
            double total_length = 0.0;

            for (std::size_t i = 1; i < points_.size(); ++i)
            {
                total_length += vsg::length(points_[i] - points_[i - 1]);
            }

            ImGui::Text("Всего (по опорным): %.3f м", total_length);
        }
    }

    ImGui::InputText("Имя пути", track_name_, sizeof(track_name_));

    ImGui::BeginDisabled(points_.size() < 2);

    if (ImGui::Button("Создать путь"))
    {
        if (create_track())
        {
            clear();
        }
    }

    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Сброс"))
    {
        clear();
    }

    ImGui::End();
}

bool SplineTool::create_track()
{
    const std::string name(track_name_);

    if (name.empty())
    {
        context_.status = "Новый путь: задайте имя";
        return false;
    }

    if (points_.size() < 2)
    {
        context_.status = "Новый путь: нужно минимум две точки";
        return false;
    }

    // Запись ProposedTrack в track-edit.conf + геометрия пути вдоль
    // сплайна (сэмплы Catmull-Rom с шагом 5 м)
    if (!TrackFurniture::generate_proposed_track(context_, name, points_))
    {
        return false;
    }

    // Сохранение track-edit.conf целиком (как в окне «Путь»)
    const FileSystem& fs = FileSystem::getInstance();

    const std::string track_edit_path = fs.combinePath(
        context_.route_dir, "track-edit.conf");

    if (!save_track_edit_config(track_edit_path, context_))
    {
        context_.status = "Failed to save track-edit.conf";
    }

    return true;
}
