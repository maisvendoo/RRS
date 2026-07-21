#include "CameraHandler.h"

#include "Action.h"
#include "KeyboardHandler.h"
#include "Mouse.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/app/Window.h>
#include <vsg/commands/Commands.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/ViewportState.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

static vsg::ref_ptr<vsg::Commands> create_quad(
    const vsg::dvec3& p0, const vsg::dvec3& p1,
    const vsg::dvec3& p2, const vsg::dvec3& p3
)
{
    const auto vertices = vsg::vec3Array::create(4);
    vertices->at(0) = p0;
    vertices->at(1) = p1;
    vertices->at(2) = p2;
    vertices->at(3) = p3;

    const auto indices = vsg::ushortArray::create({
        0, 1, 2,
        2, 1, 3
    });

    const auto vid = vsg::VertexIndexDraw::create();
    vid->assignArrays(vsg::DataList{vertices});
    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = 1;

    const auto commands = vsg::Commands::create();
    commands->addChild(vid);

    return commands;
}

static int get_binding_state(vsg::ref_ptr<KeyboardHandler> keyboard_handler,
    Action action)
{
    return static_cast<int>(keyboard_handler->get_binding_state(action));
}

CameraHandler::CameraHandler(
    const camera_settings_t& camera_settings,
    vsg::ref_ptr<vsg::Camera>& camera,
    VkExtent2D window_extent,
    vsg::ref_ptr<Mouse>& mouse,
    vsg::ref_ptr<KeyboardHandler>& keyboard_handler,
    double& delta_time
)
    : camera_settings(camera_settings)
    , camera(camera)
    , mouse(mouse)
    , keyboard_handler(keyboard_handler)
    , delta_time(delta_time)
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

    constexpr double scale = 50.0;

    orthographic = vsg::Orthographic::create(
        -1.0 * scale * aspect_ratio, 1.0 * scale * aspect_ratio, -1.0 * scale, 1.0 * scale,
        camera_settings.zNear, camera_settings.view_distance
    );

    const double initial_height = camera_settings.initial_height;

    look_at = vsg::LookAt::create(
        vsg::dvec3(0.0, 0.0, initial_height),
        vsg::dvec3(0.0, 1.0, initial_height),
        vsg::dvec3(0.0, 0.0, 1.0));

    camera = vsg::Camera::create(perspective, look_at,
        vsg::ViewportState::create(window_extent));

    calculate_front();
    calculate_right();
    calculate_up();
}

void CameraHandler::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled || !mouse->is_rmb_pressed())
    {
        return;
    }

    const vsg::ivec2 delta_mouse_pos = mouse->get_delta_pos();
    const double rotate_speed = camera_settings.rotate_speed;

    yaw_deg_ += delta_mouse_pos.x * rotate_speed;
    pitch_deg_ -= delta_mouse_pos.y * rotate_speed;
    pitch_deg_ = std::clamp(pitch_deg_, -89.0, 89.0);

    calculate_front();
    calculate_right();
    calculate_up();
}

void CameraHandler::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled)
    {
        return;
    }

    const double zoom_power = camera_settings.zoom_power;

    double& fovy = perspective->fieldOfViewY;
    fovy -= scrollWheel.delta.y * zoom_power;
    fovy = std::clamp(fovy, camera_settings.fovy_min, camera_settings.fovy_max);
}

void CameraHandler::apply(vsg::FrameEvent&)
{
    if (!mouse->is_rmb_pressed())
    {
        return;
    }

    const double move_speed = camera_settings.move_speed *
        delta_time;

    look_at->eye += front_ * move_speed * static_cast<double>(
        get_binding_state(keyboard_handler, ACTION_MOVE_CAMERA_FORWARD) -
        get_binding_state(keyboard_handler, ACTION_MOVE_CAMERA_BACKWARD));

    look_at->eye += right_ * move_speed * static_cast<double>(
        get_binding_state(keyboard_handler, ACTION_MOVE_CAMERA_RIGHT) -
        get_binding_state(keyboard_handler, ACTION_MOVE_CAMERA_LEFT));

    if (get_binding_state(keyboard_handler, ACTION_CHANGE_PROJECTION_MATRIX))
    {
        if (camera->projectionMatrix == perspective)
        {
            camera->projectionMatrix = orthographic;
        }
        else
        {
            camera->projectionMatrix = perspective;
        }
    }

    look_at->center = look_at->eye + front_;
}

const vsg::dvec3& CameraHandler::get_front() const
{
    return front_;
}

const vsg::dvec3& CameraHandler::get_right() const
{
    return right_;
}

const vsg::dvec3& CameraHandler::get_up() const
{
    return up_;
}

// Create plane perpedicular to camera normal passing through
// specified point to test for intersections
vsg::ref_ptr<vsg::Node> CameraHandler::create_front_plane(
    const vsg::dvec3& point,
    vsg::dvec3* up_out
) const
{
    constexpr double angle_rad = vsg::radians(80.0);

    const vsg::dvec3& camera_pos = look_at->eye;

    const auto get_dir = [&](int yaw_dir, int pitch_dir) -> vsg::dvec3
    {
        const double yaw_angle_rad = angle_rad * yaw_dir;
        const double pitch_angle_rad = angle_rad * pitch_dir;

        return vsg::rotate(yaw_angle_rad, up_) *
            vsg::rotate(-pitch_angle_rad, right_) * front_;
    };

    const vsg::dvec3 p0_dir = get_dir(-1, -1);
    const vsg::dvec3 p1_dir = get_dir( 1, -1);
    const vsg::dvec3 p2_dir = get_dir(-1,  1);
    const vsg::dvec3 p3_dir = get_dir( 1,  1);

    const vsg::dvec3 camera_to_point = point - camera_pos;

    const double camera_norm_length = vsg::length(camera_to_point) *
        vsg::dot(front_, vsg::normalize(camera_to_point));

    const double dist = camera_norm_length / vsg::dot(front_, p0_dir);

    const vsg::dvec3 p0 = camera_pos + p0_dir * dist;
    const vsg::dvec3 p1 = camera_pos + p1_dir * dist;
    const vsg::dvec3 p2 = camera_pos + p2_dir * dist;
    const vsg::dvec3 p3 = camera_pos + p3_dir * dist;

    if (up_out)
    {
        *up_out = vsg::normalize(p2 - p0);
    }

    return create_quad(p0, p1, p2, p3);
}

void CameraHandler::calculate_front()
{
    const double yaw_rad = vsg::radians(yaw_deg_);
    const double pitch_rad = vsg::radians(pitch_deg_);

    front_ = vsg::normalize(vsg::dvec3(
        sin(yaw_rad) * cos(pitch_rad),
        cos(yaw_rad) * cos(pitch_rad),
        sin(pitch_rad)
    ));
}

void CameraHandler::calculate_right()
{
    right_ = vsg::normalize(vsg::cross(front_, look_at->up));
}

void CameraHandler::calculate_up()
{
    up_ = vsg::normalize(vsg::cross(right_, front_));
}
