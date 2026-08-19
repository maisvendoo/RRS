#include "editor/EventHandler.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/IntersectionHandler.h"
#include "editor/MouseButton.h"
#include "editor/ObjectSelector.h"
#include "editor/RouteObject.h"
#include "editor/commands/CommandList.h"
#include "editor/commands/DeleteObjects.h"
#include "editor/settings/CameraSettings.h"
#include "editor/states/EditorState.h"

#include <Journal.h>
#include <filesystem.h>

#include <vsgImGui/imgui.h>

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
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
        (keyPress.mask & vsg::MODKEY_Shift) != 0;

    handle_shortcuts(keyPress);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    keyboard->apply(keyRelease);

    context_.shift_pressed =
        (keyRelease.mask & vsg::MODKEY_Shift) != 0;
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
    context_.object_selector->apply(buttonPress);
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

    const bool ctrl_pressed =
        (keyPress.mask & vsg::MODKEY_Control) != 0;

    if (ctrl_pressed)
    {
        switch (keyPress.keyBase)
        {
            case vsg::KEY_z:
            case vsg::KEY_Z:
            {
                if (keyPress.mask & vsg::MODKEY_Shift)
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
            case vsg::KEY_y:
            case vsg::KEY_Y:
            {
                context_.commands.redo();
                context_.status = "Redo";
                return;
            }
            case vsg::KEY_s:
            case vsg::KEY_S:
            {
                save_route();
                return;
            }
            default:
            {
                return;
            }
        }
    }

    switch (keyPress.keyBase)
    {
        case vsg::KEY_Delete:
        case vsg::KEY_KP_Delete:
        {
            if (!context_.selected_objects.empty())
            {
                context_.commands.push(new DeleteObjects(context_), true);
                context_.status = "Objects deleted";
            }

            return;
        }
        default:
        {
            return;
        }
    }
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
        static_cast<double>(keyboard->pressed(vsg::KEY_W, false)) -
        static_cast<double>(keyboard->pressed(vsg::KEY_S, false));

    const double right_state =
        static_cast<double>(keyboard->pressed(vsg::KEY_D, false)) -
        static_cast<double>(keyboard->pressed(vsg::KEY_A, false));

    // Q/E — вниз/вверх вдоль мировой вертикали
    const double up_state =
        static_cast<double>(keyboard->pressed(vsg::KEY_E, false)) -
        static_cast<double>(keyboard->pressed(vsg::KEY_Q, false));

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

    context_.status = "Route saved: " + route_map_path;
}
