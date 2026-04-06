#include "CameraHandler.h"

#include "Action.h"
#include "EditorContext.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"
#include "Settings.h"

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

using vec2_type = vsg::t_vec2<CameraHandler::value_type>;
using vec3_array_type = vsg::Array<CameraHandler::vec3_type>;
using look_at_vec3_type = vsg::t_vec3<double>;

static vsg::ref_ptr<vsg::Commands> create_quad(
    const CameraHandler::vec3_type& p0,
    const CameraHandler::vec3_type& p1,
    const CameraHandler::vec3_type& p2,
    const CameraHandler::vec3_type& p3
)
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

CameraHandler::CameraHandler(EditorContext& context)
    : context(context)
{
    const settings_t& settings = context.settings;
    const VkExtent2D window_extent = context.window->extent2D();

    context.perspective = vsg::Perspective::create(
        static_cast<double>(settings.fovy),
        static_cast<double>(window_extent.width) /
            static_cast<double>(window_extent.height),
        static_cast<double>(settings.zNear),
        static_cast<double>(settings.view_distance));

    const double initial_height =
        static_cast<double>(settings.camera_initial_height);

    context.look_at = vsg::LookAt::create(
        vsg::dvec3(0.0, 0.0, initial_height),
        vsg::dvec3(0.0, 1.0, initial_height),
        vsg::dvec3(0.0, 0.0, 1.0));

    context.camera = vsg::Camera::create(context.perspective, context.look_at,
        vsg::ViewportState::create(window_extent));

    calculate_front();
    calculate_right();
    calculate_up();
}

void CameraHandler::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled)
    {
        return;
    }

    if (context.mouse_handler->get_is_rmb_pressed())
    {
        const vec2_type delta_mouse_pos = static_cast<vec2_type>(
            context.mouse_handler->get_delta_pos());

        const value_type rotate_speed =
            static_cast<value_type>(context.settings.camera_rotate_speed) *
            static_cast<value_type>(context.delta_time);

        yaw_deg += delta_mouse_pos.x * rotate_speed;

        pitch_deg = std::clamp(
            pitch_deg - delta_mouse_pos.y * rotate_speed,
            static_cast<value_type>(-89.0f),
            static_cast<value_type>(89.0f));

        calculate_front();
        calculate_right();
        calculate_up();
    }
}

void CameraHandler::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled)
    {
        return;
    }

    const settings_t& settings = context.settings;

    const double zoom_power = static_cast<double>(settings.camera_zoom_power) *
        context.delta_time;

    const auto perspective = context.perspective;

    perspective->fieldOfViewY = std::clamp(
        perspective->fieldOfViewY - scrollWheel.delta.y * zoom_power,
        static_cast<double>(settings.fovy_min),
        static_cast<double>(settings.fovy_max));
}

void CameraHandler::apply(vsg::FrameEvent& frame)
{
    (void)frame;

    if (!context.mouse_handler->get_is_rmb_pressed())
    {
        return;
    }

    const auto look_at = context.look_at;

    const look_at_vec3_type front =
        static_cast<look_at_vec3_type>(this->front);

    const look_at_vec3_type right =
            static_cast<look_at_vec3_type>(this->right);

    const double move_speed =
        static_cast<double>(context.settings.camera_move_speed) *
        context.delta_time;

    const auto keyboard_handler = context.keyboard_handler;

    const auto get_binding_state = [keyboard_handler](Action action) -> int
    {
        return static_cast<int>(keyboard_handler->get_binding_state(action));
    };

    look_at->eye += front * move_speed * static_cast<double>(
        get_binding_state(ACTION_MOVE_CAMERA_FORWARD) -
        get_binding_state(ACTION_MOVE_CAMERA_BACKWARD));

    look_at->eye += right * move_speed * static_cast<double>(
        get_binding_state(ACTION_MOVE_CAMERA_RIGHT) -
        get_binding_state(ACTION_MOVE_CAMERA_LEFT));

    look_at->center = look_at->eye + front;
}

const vsg::dvec3& CameraHandler::get_front() const
{
    return front;
}

const vsg::dvec3& CameraHandler::get_right() const
{
    return right;
}

const vsg::dvec3& CameraHandler::get_up() const
{
    return up;
}

// Create plane perpedicular to camera normal passing through
// specified point to test for intersections
vsg::ref_ptr<vsg::Node> CameraHandler::create_front_plane(
    const vec3_type& point,
    vec3_type* up_out
) const
{
    constexpr value_type angle_rad = vsg::radians(static_cast<value_type>(80));

    const vec3_type camera_pos = static_cast<vec3_type>(context.look_at->eye);

    const auto get_dir = [&](int yaw_dir, int pitch_dir) -> vec3_type
    {
        const value_type yaw_angle_rad = angle_rad *
            static_cast<value_type>(yaw_dir);

        const value_type pitch_angle_rad = angle_rad *
            static_cast<value_type>(pitch_dir);

        return vsg::rotate(yaw_angle_rad, static_cast<vsg::vec3>(up)) *
            vsg::rotate(-pitch_angle_rad, static_cast<vsg::vec3>(right)) *
            static_cast<vsg::vec3>(front);
    };

    const vec3_type p0_dir = get_dir(-1, -1);
    const vec3_type p1_dir = get_dir( 1, -1);
    const vec3_type p2_dir = get_dir(-1,  1);
    const vec3_type p3_dir = get_dir( 1,  1);

    const vec3_type camera_to_point = point - camera_pos;

    const value_type camera_norm_length = vsg::length(camera_to_point) *
        vsg::dot(static_cast<vsg::vec3>(front),
        vsg::normalize(camera_to_point));

    const value_type dist = camera_norm_length /
        vsg::dot(static_cast<vsg::vec3>(front), p0_dir);

    const vsg::vec3 p0 = camera_pos + p0_dir * dist;
    const vsg::vec3 p1 = camera_pos + p1_dir * dist;
    const vsg::vec3 p2 = camera_pos + p2_dir * dist;
    const vsg::vec3 p3 = camera_pos + p3_dir * dist;

    if (up_out)
    {
        *up_out = vsg::normalize(p2 - p0);
    }

    return create_quad(p0, p1, p2, p3);
}

void CameraHandler::calculate_front()
{
    const value_type yaw_rad = vsg::radians(yaw_deg);
    const value_type pitch_rad = vsg::radians(pitch_deg);

    front = vsg::normalize(vec3_type(
        std::sin(yaw_rad) * std::cos(pitch_rad),
        std::cos(yaw_rad) * std::cos(pitch_rad),
        std::sin(pitch_rad)
    ));
}

void CameraHandler::calculate_right()
{
    right = vsg::normalize(vsg::cross(front, context.look_at->up));
}

void CameraHandler::calculate_up()
{
    up = vsg::normalize(vsg::cross(right, front));
}
