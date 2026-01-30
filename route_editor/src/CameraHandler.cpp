#include "CameraHandler.h"

#include "Action.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "Settings.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/state/ViewportState.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/FrameStamp.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cassert>
#include <cmath>

CameraHandler::CameraHandler(
    const settings_t& settings,
    const VkExtent2D& window_extent,
    vsg::ref_ptr<MouseHandler> mouse_handler,
    vsg::ref_ptr<KeyboardHandler> keyboard_handler,
    double initial_height
)
    : settings(settings)
    , mouse_handler(mouse_handler)
    , keyboard_handler(keyboard_handler)
{
    assert(window_extent.width != 0);
    assert(window_extent.height != 0);
    assert(mouse_handler);
    assert(keyboard_handler);

    const double window_width = static_cast<double>(window_extent.width);
    const double window_height = static_cast<double>(window_extent.height);
    const double aspect_ratio = window_width / window_height;

    perspective = vsg::Perspective::create(settings.fovy, aspect_ratio,
        settings.zNear, settings.view_distance);

    look_at = vsg::LookAt::create();
    look_at->eye.z = look_at->center.z = initial_height;

    camera = vsg::Camera::create(perspective, look_at,
        vsg::ViewportState::create(window_extent));

    const double yaw_rad = vsg::radians(yaw_deg);
    const double pitch_rad = vsg::radians(pitch_deg);

    front.x = std::sin(yaw_rad) * std::cos(pitch_rad);
    front.y = std::cos(yaw_rad) * std::cos(pitch_rad);
    front.z = std::sin(pitch_rad);
    front = vsg::normalize(front);

    right = vsg::cross(front, look_at->up);
    right = vsg::normalize(right);
}

void CameraHandler::apply(vsg::FrameEvent& frame)
{
    static double prev_time = frame.frameStamp->simulationTime;

    const double time = frame.frameStamp->simulationTime;
    const double delta_time = time - prev_time;

    prev_time = time;

    if (mouse_handler->get_is_rmb_pressed())
    {
        const vsg::ivec2 delta_mouse_pos = mouse_handler->get_delta_pos();

        yaw_deg += delta_mouse_pos.x * settings.camera_rotate_speed *
            delta_time;

        pitch_deg -= delta_mouse_pos.y * settings.camera_rotate_speed *
            delta_time;

        pitch_deg = std::clamp(pitch_deg, settings.pitch_min,
            settings.pitch_max);

        const double yaw_rad = vsg::radians(yaw_deg);
        const double pitch_rad = vsg::radians(pitch_deg);

        front.x = std::sin(yaw_rad) * std::cos(pitch_rad);
        front.y = std::cos(yaw_rad) * std::cos(pitch_rad);
        front.z = std::sin(pitch_rad);
        front = vsg::normalize(front);

        right = vsg::cross(front, look_at->up);
        right = vsg::normalize(right);
    }

    perspective->fieldOfViewY -= mouse_handler->get_scroll() *
        settings.camera_zoom_power * delta_time;

    perspective->fieldOfViewY = std::clamp(perspective->fieldOfViewY,
        settings.fovy_min, settings.fovy_max);

    const int move_forward = static_cast<int>(
        keyboard_handler->get_binding_state(ACTION_MOVE_CAMERA_FORWARD));

    const int move_backward = static_cast<int>(
        keyboard_handler->get_binding_state(ACTION_MOVE_CAMERA_BACKWARD));

    const int move_left = static_cast<int>(
        keyboard_handler->get_binding_state(ACTION_MOVE_CAMERA_LEFT));

    const int move_right = static_cast<int>(
        keyboard_handler->get_binding_state(ACTION_MOVE_CAMERA_RIGHT));

    const vsg::dvec3 front_movememt = front * settings.camera_move_speed *
        delta_time * static_cast<double>(move_forward - move_backward);

    const vsg::dvec3 right_movement = right * settings.camera_move_speed *
        delta_time * static_cast<double>(move_right - move_left);

    look_at->eye += front_movememt;
    look_at->eye += right_movement;
    look_at->center = look_at->eye + front;
}

vsg::ref_ptr<vsg::Perspective> CameraHandler::get_perspective() const
{
    return perspective;
}

vsg::ref_ptr<vsg::LookAt> CameraHandler::get_look_at() const
{
    return look_at;
}

vsg::ref_ptr<vsg::Camera> CameraHandler::get_camera() const
{
    return camera;
}

vsg::dvec3 CameraHandler::get_front() const
{
    return front;
}

vsg::dvec3 CameraHandler::get_right() const
{
    return right;
}
