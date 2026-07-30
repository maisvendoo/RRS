#include "Camera.h"

#include "Action.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "settings/CameraSettings.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/state/ViewportState.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cmath>

Camera::Camera(
    const camera_settings_t& camera_settings,
    const VkExtent2D& window_extent,
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : camera_settings(camera_settings)
    , mouse(mouse)
    , keyboard(keyboard)
{
    const double window_width = static_cast<double>(window_extent.width);
    const double window_height = static_cast<double>(window_extent.height);
    const double aspect_ratio = window_width / window_height;

    perspective = vsg::Perspective::create(
        camera_settings.fovy,
        aspect_ratio,
        camera_settings.zNear,
        camera_settings.view_distance
    );

    create_orthographic_projection(window_width, window_height, aspect_ratio);

    const double initial_height = camera_settings.initial_height;

    look_at = vsg::LookAt::create(
        vsg::dvec3(0.0, 0.0, initial_height),
        vsg::dvec3(0.0, 1.0, initial_height),
        vsg::dvec3(0.0, 0.0, 1.0));

    this->projectionMatrix = perspective;
    this->viewMatrix = look_at;
    this->viewportState = vsg::ViewportState::create(window_extent);

    another_projection_matrix = orthographic;

    calculate_front();
    calculate_right();
    calculate_up();
    calculate_inverse_projection_matrix();
    calculate_inverse_view_matrix();
}

void Camera::handle_key_press()
{
    if (keyboard->pressed_once(ACTION_CHANGE_PROJECTION_MATRIX))
    {
        std::swap(projectionMatrix, another_projection_matrix);
        calculate_inverse_projection_matrix();
    }
}

void Camera::update_move_direction()
{
    const int forward_move_direction =
        static_cast<int>(keyboard->pressed(ACTION_MOVE_CAMERA_FORWARD)) -
        static_cast<int>(keyboard->pressed(ACTION_MOVE_CAMERA_BACKWARD));

    const int right_move_direction =
        static_cast<int>(keyboard->pressed(ACTION_MOVE_CAMERA_RIGHT)) -
        static_cast<int>(keyboard->pressed(ACTION_MOVE_CAMERA_LEFT));

    if ((forward_move_direction == 0) && (right_move_direction == 0))
    {
        move_direction = {0.0, 0.0, 0.0};
    }
    else
    {
        move_direction = front * forward_move_direction +
            right * right_move_direction;

        if (projectionMatrix == orthographic)
        {
            move_direction.z = 0.0;
        }

        move_direction = vsg::normalize(move_direction);
    }
}

void Camera::handle_mouse_move()
{
    const double rotate_speed = camera_settings.rotate_speed;

    yaw_degrees += mouse->get_delta_x() * rotate_speed;
    pitch_degrees -= mouse->get_delta_y() * rotate_speed;
    pitch_degrees = std::clamp(pitch_degrees, -89.0, 89.0);

    calculate_front();
    calculate_right();
    calculate_up();

    update_move_direction();
}

void Camera::handle_mouse_scroll()
{
    double& fovy = perspective->fieldOfViewY;
    fovy -= mouse->get_scroll() * camera_settings.zoom_power;
    fovy = std::clamp(fovy, camera_settings.fovy_min, camera_settings.fovy_max);
    calculate_inverse_projection_matrix();
}

void Camera::update(double delta_time)
{
    look_at->eye += camera_settings.move_speed * delta_time * move_direction;
    look_at->center = look_at->eye + front;
    calculate_inverse_view_matrix();
}

const vsg::dvec3& Camera::get_front() const
{
    return front;
}

const vsg::dvec3& Camera::get_right() const
{
    return right;
}

const vsg::dvec3& Camera::get_up() const
{
    return up;
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

const vsg::dmat4& Camera::get_inverse_projection_matrix() const
{
    return inverse_projection_matrix;
}

const vsg::dmat4& Camera::get_inverse_view_matrix() const
{
    return inverse_view_matrix;
}

void Camera::create_orthographic_projection(double window_width,
    double window_height, double aspect_ratio)
{
    const double radius = camera_settings.view_distance;

    const double halfDim = 100.0;
    double halfHeight, halfWidth;
    if (window_width > window_height)
    {
        halfHeight = halfDim;
        halfWidth = halfDim * aspect_ratio;
    }
    else
    {
        halfWidth = halfDim;
        halfHeight = halfDim / aspect_ratio;
    }

    orthographic = vsg::Orthographic::create(-halfWidth, halfWidth,
        -halfHeight, halfHeight, 0.0, radius);
}

void Camera::calculate_front()
{
    const double yaw_radians = vsg::radians(yaw_degrees);
    const double pitch_radians = vsg::radians(pitch_degrees);

    front = vsg::normalize(vsg::dvec3(
        std::sin(yaw_radians) * std::cos(pitch_radians),
        std::cos(yaw_radians) * std::cos(pitch_radians),
        std::sin(pitch_radians)
    ));
}

void Camera::calculate_right()
{
    right = vsg::normalize(vsg::cross(front, look_at->up));
}

void Camera::calculate_up()
{
    up = vsg::normalize(vsg::cross(right, front));
}

void Camera::calculate_inverse_projection_matrix()
{
    inverse_projection_matrix = projectionMatrix->inverse();
}

void Camera::calculate_inverse_view_matrix()
{
    inverse_view_matrix = look_at->inverse();
}
