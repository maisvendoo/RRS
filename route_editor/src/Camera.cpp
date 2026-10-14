#include "editor/Camera.h"

#include "editor/Action.h"
#include "editor/EditorContext.h"
#include "editor/Keyboard.h"
#include "editor/Mouse.h"

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
#include <cassert>
#include <cmath>

Camera::Camera(EditorContext& editor_context)
    : editor_context(editor_context)
{
    assert(editor_context.window && "Window must be created before camera");

    const auto& camera_settings = editor_context.camera_settings;
    const auto& window_extent = editor_context.window->extent2D();

    const double window_width = static_cast<double>(window_extent.width);
    const double window_height = static_cast<double>(window_extent.height);
    const double aspect_ratio = window_width / window_height;

    perspective = vsg::Perspective::create(camera_settings.fovy, aspect_ratio,
                                           camera_settings.zNear,
                                           camera_settings.view_distance);

    create_orthographic_projection(aspect_ratio);

    const double initial_height = camera_settings.initial_height;

    look_at = vsg::LookAt::create(vsg::dvec3(0.0, 0.0, initial_height),
                                  vsg::dvec3(0.0, 1.0, initial_height),
                                  vsg::dvec3(0.0, 0.0, 1.0));

    this->projectionMatrix = perspective;
    this->viewMatrix = look_at;
    this->viewportState = vsg::ViewportState::create(window_extent);

    another_proj_mat = orthographic;

    calc_front();
    calc_right();
    calc_up();
    calc_inv_proj_mat();
    calc_inv_view_mat();
}

void Camera::swap_proj_mat()
{
    std::swap(projectionMatrix, another_proj_mat);
    calc_inv_proj_mat();
}

void Camera::update_move_dir()
{
    const auto& keyboard = editor_context.keyboard;

    const int forward_move_dir
        = static_cast<int>(keyboard->pressed(ACTION_MOVE_CAMERA_FORWARD))
          - static_cast<int>(keyboard->pressed(ACTION_MOVE_CAMERA_BACKWARD));

    const int right_move_dir
        = static_cast<int>(keyboard->pressed(ACTION_MOVE_CAMERA_RIGHT))
          - static_cast<int>(keyboard->pressed(ACTION_MOVE_CAMERA_LEFT));

    if ((forward_move_dir == 0) && (right_move_dir == 0))
    {
        move_dir = {0.0, 0.0, 0.0};
        return;
    }

    move_dir = front * forward_move_dir + right * right_move_dir;

    if (projectionMatrix == orthographic)
    {
        move_dir.z = 0.0;
    }

    move_dir = vsg::normalize(move_dir);
}

void Camera::handle_mouse_move()
{
    const auto& camera_settings = editor_context.camera_settings;
    const auto& mouse = editor_context.mouse;

    const double rotate_speed = camera_settings.rotate_speed;

    yaw_deg += mouse->get_delta_x() * rotate_speed;
    pitch_deg -= mouse->get_delta_y() * rotate_speed;
    pitch_deg = std::clamp(pitch_deg, -89.0, 89.0);

    calc_front();
    calc_right();
    calc_up();

    update_move_dir();
}

void Camera::handle_mouse_scroll()
{
    const auto& camera_settings = editor_context.camera_settings;
    const auto& mouse = editor_context.mouse;

    double& fovy = perspective->fieldOfViewY;
    fovy -= mouse->get_scroll() * camera_settings.zoom_power;
    fovy = std::clamp(fovy, camera_settings.fovy_min, camera_settings.fovy_max);
    calc_inv_proj_mat();
}

void Camera::update(double delta_time)
{
    const auto& camera_settings = editor_context.camera_settings;

    look_at->eye += camera_settings.move_speed * delta_time * move_dir;
    look_at->center = look_at->eye + front;
    calc_inv_view_mat();
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

const vsg::dmat4& Camera::get_inv_proj_mat() const
{
    return inv_proj_mat;
}

const vsg::dmat4& Camera::get_inv_view_mat() const
{
    return inv_view_mat;
}

void Camera::look_on(const vsg::dvec3& pos, double distance)
{
    look_at->eye = pos - front * distance;
    look_at->center = look_at->eye + front;
}

void Camera::create_orthographic_projection(double aspect_ratio)
{
    const auto& camera_settings = editor_context.camera_settings;

    const double radius = camera_settings.view_distance;

    const double halfDim = 100.0;
    double halfHeight, halfWidth;
    if (aspect_ratio > 1.0f)
    {
        halfHeight = halfDim;
        halfWidth = halfDim * aspect_ratio;
    }
    else
    {
        halfWidth = halfDim;
        halfHeight = halfDim / aspect_ratio;
    }

    orthographic = vsg::Orthographic::create(-halfWidth, halfWidth, -halfHeight,
                                             halfHeight, 0.0, radius);
}

void Camera::calc_front()
{
    const double yaw_rad = vsg::radians(yaw_deg);
    const double pitch_rad = vsg::radians(pitch_deg);

    front = vsg::normalize(vsg::dvec3(std::sin(yaw_rad) * std::cos(pitch_rad),
                                      std::cos(yaw_rad) * std::cos(pitch_rad),
                                      std::sin(pitch_rad)));
}

void Camera::calc_right()
{
    right = vsg::normalize(vsg::cross(front, look_at->up));
}

void Camera::calc_up()
{
    up = vsg::normalize(vsg::cross(right, front));
}

void Camera::calc_inv_proj_mat()
{
    inv_proj_mat = projectionMatrix->inverse();
}

void Camera::calc_inv_view_mat()
{
    inv_view_mat = look_at->inverse();
}
