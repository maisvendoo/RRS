#include "editor/Camera.h"

#include "editor/Action.h"
#include "editor/Keyboard.h"
#include "editor/Mouse.h"
#include "editor/check_initialization.h"
#include "editor/settings/CameraSettings.h"

#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/vec3.h>
#include <vsg/state/ViewportState.h>
#include <vsg/ui/PointerEvent.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cmath>

Camera::Camera(
    const camera_settings_t& camera_settings,
    VkExtent2D window_extent,
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : camera_settings(camera_settings)
    , mouse(mouse)
    , keyboard(keyboard)
{
    create_perspective(window_extent);
    create_orthographic();
    create_look_at();
    create_viewport_state(window_extent);

    projectionMatrix = perspective;
    viewMatrix = look_at;
}

Camera::~Camera() = default;

void Camera::update_move_direction()
{
    const double forward_move_direction =
        static_cast<double>(keyboard->pressed(ACTION_MOVE_CAMERA_FORWARD)) -
        static_cast<double>(keyboard->pressed(ACTION_MOVE_CAMERA_BACKWARD));

    const double right_move_direction =
        static_cast<double>(keyboard->pressed(ACTION_MOVE_CAMERA_RIGHT)) -
        static_cast<double>(keyboard->pressed(ACTION_MOVE_CAMERA_LEFT));

    if ((std::abs(forward_move_direction) > 1.0e-6) ||
        (std::abs(right_move_direction) > 1.0e-6))
    {
        move_direction = vsg::normalize(front * forward_move_direction +
            right * right_move_direction);
    }
    else
    {
        move_direction = {0.0, 0.0, 0.0};
    }
}

void Camera::handle_mouse_move()
{
    yaw_degrees += camera_settings.rotate_speed * mouse->get_delta_x();
    pitch_degrees -= camera_settings.rotate_speed * mouse->get_delta_y();
    pitch_degrees = std::clamp(pitch_degrees, -89.0, 89.0);

    const double yaw_radians = vsg::radians(yaw_degrees);
    const double pitch_radians = vsg::radians(pitch_degrees);

    front = vsg::normalize(vsg::dvec3(
        std::sin(yaw_radians) * std::cos(pitch_radians),
        std::cos(yaw_radians) * std::cos(pitch_radians),
        std::sin(pitch_radians)
    ));

    right = vsg::normalize(vsg::cross(front, look_at->up));

    update_move_direction();
}

void Camera::update(double delta_time)
{
    look_at->eye += camera_settings.move_speed * delta_time * move_direction;
    look_at->center = look_at->eye + front;
}

const vsg::ref_ptr<vsg::Perspective>& Camera::get_perspective() const
{
    return perspective;
}

const vsg::ref_ptr<vsg::Orthographic>& Camera::get_orthographic() const
{
    return orthographic;
}

const vsg::ref_ptr<vsg::LookAt>& Camera::get_look_at() const
{
    return look_at;
}

void Camera::create_perspective(VkExtent2D window_extent)
{
    perspective = vsg::Perspective::create(
        camera_settings.fovy_degrees,
        static_cast<double>(window_extent.width) /
            static_cast<double>(window_extent.height),
        camera_settings.zNear,
        camera_settings.view_distance
    );
    CHECK_INITIALIZATION(perspective);
}

void Camera::create_orthographic()
{
    orthographic = vsg::Orthographic::create(-1.0, 1.0, -1.0, 1.0,
        camera_settings.zNear, camera_settings.view_distance);
    CHECK_INITIALIZATION(orthographic);
}

void Camera::create_look_at()
{
    look_at = vsg::LookAt::create();
    CHECK_INITIALIZATION(look_at);

    look_at->eye.z = look_at->center.z = camera_settings.initial_height;

    front = look_at->center - look_at->eye;
    right = vsg::cross(front, look_at->up);
}

void Camera::create_viewport_state(VkExtent2D window_extent)
{
    viewportState = vsg::ViewportState::create(window_extent);
    CHECK_INITIALIZATION(viewportState);
}
