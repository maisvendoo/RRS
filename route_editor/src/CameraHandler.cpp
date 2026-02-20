#include "CameraHandler.h"

#include "Action.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "Settings.h"

#include <vsg/app/Camera.h>
#include <vsg/app/ProjectionMatrix.h>
#include <vsg/app/ViewMatrix.h>
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
#include <vsg/ui/FrameStamp.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

using vec2_type = vsg::t_vec2<CameraHandler::value_type>;
using vec3_array_type = vsg::Array<CameraHandler::vec3_type>;
using perspective_value_type = decltype(vsg::Perspective::fieldOfViewY);
using look_at_value_type = decltype(vsg::LookAt::eye.x);
using look_at_vec3_type = vsg::t_vec3<look_at_value_type>;
using time_type = decltype(vsg::FrameEvent::frameStamp->simulationTime);

static vsg::ref_ptr<vsg::Commands> create_quad(
    const CameraHandler::vec3_type& p0, const CameraHandler::vec3_type& p1,
    const CameraHandler::vec3_type& p2, const CameraHandler::vec3_type& p3)
{
    const auto vertices = vec3_array_type::create({p0, p1, p2, p3});

    const auto indices = vsg::ushortArray::create({
        0, 1, 2,
        2, 1, 3
    });

    const auto vid = vsg::VertexIndexDraw::create();
    vid->assignArrays(vsg::DataList{vertices});
    vid->assignIndices(indices);
    vid->indexCount = static_cast<std::uint32_t>(indices->size());
    vid->instanceCount = 1;

    const auto commands = vsg::Commands::create();
    commands->addChild(vid);

    return commands;
}

CameraHandler::CameraHandler(
    const settings_t& settings,
    const VkExtent2D& window_extent,
    vsg::ref_ptr<MouseHandler> mouse_handler,
    vsg::ref_ptr<KeyboardHandler> keyboard_handler
)
    : settings(settings)
    , mouse_handler(mouse_handler)
    , keyboard_handler(keyboard_handler)
{
    assert(window_extent.width != 0);
    assert(window_extent.height != 0);
    assert(mouse_handler);
    assert(keyboard_handler);

    const auto fovy = static_cast<perspective_value_type>(settings.fovy);

    const auto window_width = static_cast<perspective_value_type>(
        window_extent.width);

    const auto window_height = static_cast<perspective_value_type>(
        window_extent.height);

    const perspective_value_type aspect_ratio = window_width / window_height;

    const auto near_distance = static_cast<perspective_value_type>(
        settings.zNear);

    const auto far_distance = static_cast<perspective_value_type>(
        settings.view_distance);

    perspective = vsg::Perspective::create(fovy, aspect_ratio,
        near_distance, far_distance);

    const auto initial_height = static_cast<look_at_value_type>(
        settings.camera_initial_height);

    look_at = vsg::LookAt::create();
    look_at->eye.z = look_at->center.z = initial_height;

    camera = vsg::Camera::create(perspective, look_at,
        vsg::ViewportState::create(window_extent));

    calculate_front();
    calculate_right();
    calculate_up();
}

void CameraHandler::apply(vsg::FrameEvent& frame)
{
    static time_type prev_time = frame.frameStamp->simulationTime;

    const time_type time = frame.frameStamp->simulationTime;
    const time_type delta_time = time - prev_time;

    prev_time = time;

    if (mouse_handler->get_is_rmb_pressed())
    {
        const auto delta_mouse_pos = static_cast<vec2_type>(
            mouse_handler->get_delta_pos());

        const auto rotate_speed = static_cast<value_type>(
            settings.camera_rotate_speed);

        yaw_deg += delta_mouse_pos.x * rotate_speed *
            static_cast<value_type>(delta_time);

        pitch_deg -= delta_mouse_pos.y * rotate_speed *
            static_cast<value_type>(delta_time);

        const auto pitch_min = static_cast<value_type>(settings.pitch_min);
        const auto pitch_max = static_cast<value_type>(settings.pitch_max);

        pitch_deg = std::clamp(pitch_deg, pitch_min, pitch_max);

        calculate_front();
        calculate_right();
        calculate_up();
    }

    if (const auto scroll = static_cast<perspective_value_type>(
        mouse_handler->get_scroll()))
    {
        const auto zoom_power = static_cast<perspective_value_type>(
            settings.camera_zoom_power);

        perspective->fieldOfViewY -= scroll * zoom_power *
            static_cast<perspective_value_type>(delta_time);

        const auto fovy_min = static_cast<perspective_value_type>(
            settings.fovy_min);

        const auto fovy_max = static_cast<perspective_value_type>(
            settings.fovy_max);

        perspective->fieldOfViewY = std::clamp(perspective->fieldOfViewY,
            fovy_min, fovy_max);
    }

    const auto get_binding_state = [this](Action action) -> int
    {
        return static_cast<int>(keyboard_handler->get_binding_state(action));
    };

    const int move_forward = get_binding_state(ACTION_MOVE_CAMERA_FORWARD);
    const int move_backward = get_binding_state(ACTION_MOVE_CAMERA_BACKWARD);
    const int move_left = get_binding_state(ACTION_MOVE_CAMERA_LEFT);
    const int move_right = get_binding_state(ACTION_MOVE_CAMERA_RIGHT);

    const auto front = static_cast<look_at_vec3_type>(this->front);
    const auto right = static_cast<look_at_vec3_type>(this->right);

    const auto move_speed = static_cast<look_at_value_type>(
        settings.camera_move_speed);

    if (move_forward - move_backward != 0)
    {
        const look_at_vec3_type front_movememt = front * move_speed *
            static_cast<look_at_value_type>(delta_time) *
            static_cast<look_at_value_type>(move_forward - move_backward);

        look_at->eye += front_movememt;
    }

    if (move_right - move_left != 0)
    {
        const look_at_vec3_type right_movement = right * move_speed *
            static_cast<look_at_value_type>(delta_time) *
            static_cast<look_at_value_type>(move_right - move_left);

        look_at->eye += right_movement;
    }

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

double& CameraHandler::get_fov_deg() const
{
    return perspective->fieldOfViewY;
}

vsg::dvec3& CameraHandler::get_eye() const
{
    return look_at->eye;
}

CameraHandler::vec3_type CameraHandler::get_front() const
{
    return front;
}

CameraHandler::vec3_type CameraHandler::get_right() const
{
    return right;
}

CameraHandler::vec3_type CameraHandler::get_up() const
{
    return up;
}

vsg::ref_ptr<vsg::Node> CameraHandler::create_front_plane(vec3_type point) const
{
    const auto camera_pos = static_cast<vec3_type>(look_at->eye);
    const auto angle_rad = vsg::radians(static_cast<value_type>(80));

    const auto get_dir = [&](int yaw_dir, int pitch_dir) -> vec3_type
    {
        const auto yaw_angle_rad = angle_rad *
            static_cast<value_type>(yaw_dir);

        const auto pitch_angle_rad = angle_rad *
            static_cast<value_type>(pitch_dir);

        return vsg::normalize(front * vsg::rotate(yaw_angle_rad, up) *
            vsg::rotate(-pitch_angle_rad, right));
    };

    const vec3_type p0_dir = get_dir(-1, -1);
    const vec3_type p1_dir = get_dir( 1, -1);
    const vec3_type p2_dir = get_dir(-1,  1);
    const vec3_type p3_dir = get_dir( 1,  1);

    const vec3_type camera_to_point = point - camera_pos;

    const value_type camera_norm_length = vsg::length(camera_to_point) *
        vsg::dot(front, vsg::normalize(camera_to_point));

    const value_type dist = camera_norm_length / vsg::dot(p0_dir, front);

    return create_quad(
        camera_pos + p0_dir * dist,
        camera_pos + p1_dir * dist,
        camera_pos + p2_dir * dist,
        camera_pos + p3_dir * dist
    );
}

void CameraHandler::calculate_front()
{
    const value_type yaw_rad = vsg::radians(yaw_deg);
    const value_type pitch_rad = vsg::radians(pitch_deg);

    front = vsg::normalize(vec3_type{
        std::sin(yaw_rad) * std::cos(pitch_rad),
        std::cos(yaw_rad) * std::cos(pitch_rad),
        std::sin(pitch_rad)
    });
}

void CameraHandler::calculate_right()
{
    const auto world_up = static_cast<vec3_type>(look_at->up);
    right = vsg::normalize(vsg::cross(front, world_up));
}

void CameraHandler::calculate_up()
{
    up = vsg::normalize(vsg::cross(right, front));
}
