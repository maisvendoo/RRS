#include "Gizmo.h"

#include "CameraHandler.h"
#include "CommandList.h"
#include "IntersectionHandler.h"
#include "Mask.h"
#include "MoveObjectsCommand.h"
#include "RouteObject.h"
#include "Settings.h"
#include "SingleSwitch.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsg/utils/ShaderSet.h>

#include <cassert>
#include <cmath>

static constexpr vsg::vec3 X_AXIS_POSITIVE = {1.0f, 0.0f, 0.0f};
static constexpr vsg::vec3 Y_AXIS_POSITIVE = {0.0f, 1.0f, 0.0f};
static constexpr vsg::vec3 Z_AXIS_POSITIVE = {0.0f, 0.0f, 1.0f};

Gizmo::Gizmo(
    const settings_t& settings,
    CommandList& commands,
    vsg::ref_ptr<CameraHandler> camera_handler,
    vsg::ref_ptr<IntersectionHandler> intersection_handler,
    const RouteObjects& selected_objects
)
    : settings(settings)
    , commands(commands)
    , camera_handler(camera_handler)
    , intersection_handler(intersection_handler)
    , selected_objects(selected_objects)
{
    assert(camera_handler);
    assert(intersection_handler);

    builder.shaderSet = vsg::createFlatShadedShaderSet();

    const vsg::vec3 arrow_x_color = settings.gizmo_arrow_x_color;
    const vsg::vec3 arrow_y_color = settings.gizmo_arrow_y_color;
    const vsg::vec3 arrow_z_color = settings.gizmo_arrow_z_color;

    const float plane_width = 100.0f;
    const float plane_opacity = 0.1f;
    const float line_thickness = 0.01f;

    vsg::StateInfo state_info;
    state_info.two_sided = true;
    state_info.blending = true;

    const auto rotate_geometry_info = [&](vsg::GeometryInfo& geometry_info,
        vsg::vec3 direction) -> void
    {
        if (vsg::length(vsg::cross(Z_AXIS_POSITIVE, direction)) > 0.001f)
        {
            const vsg::vec3 axis = vsg::cross(Z_AXIS_POSITIVE, direction);
            const float angle = std::acos(vsg::dot(Z_AXIS_POSITIVE, direction));
            geometry_info.transform = vsg::rotate(angle, axis);
        }
    };

    const auto create_arrow = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        float thickness = settings.gizmo_arrow_thickness;
        const float length = settings.gizmo_arrow_length;
        const float opacity = settings.gizmo_opacity;

        vsg::box box = {
            vsg::vec3(-thickness, -thickness, 0.0f),
            vsg::vec3( thickness,  thickness, length)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, direction);
        geometry_info.color = {color, opacity};

        const auto cylinder = builder.createCylinder(geometry_info, state_info);

        thickness *= 3.0f;

        box.min = {-thickness, -thickness, length};
        box.max = { thickness,  thickness, length + thickness * 5.0f};

        geometry_info.set(box);

        const auto cone = builder.createCone(geometry_info, state_info);

        const auto arrow = vsg::Group::create();
        arrow->addChild(cylinder);
        arrow->addChild(cone);

        return arrow;
    };

    const auto create_plane = [&](vsg::vec3 normal,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        const float width = plane_width;
        const float opacity = plane_opacity;

        const vsg::box box = {
            vsg::vec3(-width, -width, 0.0f),
            vsg::vec3( width,  width, 0.0f)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, normal);
        geometry_info.color = {color, opacity};

        return builder.createQuad(geometry_info, state_info);
    };

    const auto create_line = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        const float width = plane_width;
        const float thickness = line_thickness;
        const float opacity = settings.gizmo_opacity;

        const vsg::box box = {
            vsg::vec3(-thickness, -thickness, -width),
            vsg::vec3( thickness,  thickness,  width)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, direction);
        geometry_info.color = {color, opacity};

        return builder.createCylinder(geometry_info, state_info);
    };

    arrow_x = create_arrow(X_AXIS_POSITIVE, arrow_x_color);
    arrow_y = create_arrow(Y_AXIS_POSITIVE, arrow_y_color);
    arrow_z = create_arrow(Z_AXIS_POSITIVE, arrow_z_color);

    this->addChild(arrow_x);
    this->addChild(arrow_y);
    this->addChild(arrow_z);

    plane_yz_switch = SingleSwitch::create(vsg::MASK_OFF,
        create_plane(X_AXIS_POSITIVE, arrow_x_color));

    plane_xz_switch = SingleSwitch::create(vsg::MASK_OFF,
        create_plane(Y_AXIS_POSITIVE, arrow_y_color));

    plane_xy_switch = SingleSwitch::create(vsg::MASK_OFF,
        create_plane(Z_AXIS_POSITIVE, arrow_z_color));

    this->addChild(plane_yz_switch);
    this->addChild(plane_xz_switch);
    this->addChild(plane_xy_switch);

    line_x_switch = SingleSwitch::create(vsg::MASK_OFF,
        create_line(X_AXIS_POSITIVE, arrow_x_color));

    line_y_switch = SingleSwitch::create(vsg::MASK_OFF,
        create_line(Y_AXIS_POSITIVE, arrow_y_color));

    line_z_switch = SingleSwitch::create(vsg::MASK_OFF,
        create_line(Z_AXIS_POSITIVE, arrow_z_color));

    this->addChild(line_x_switch);
    this->addChild(line_y_switch);
    this->addChild(line_z_switch);
}

bool Gizmo::handle_intersections()
{
    const auto intersector = intersection_handler->get_lmb_intersector();
    if (!intersector)
    {
        return false;
    }

    this->accept(*intersector);

    const auto intersection = intersection_handler->get_closest_intersection(
        intersector);

    if (!intersection)
    {
        return false;
    }

    const auto world_intersection = static_cast<vsg::vec3>(
        intersection->worldIntersection);

    const auto& node_path = intersection->nodePath;

    const auto camera_front = static_cast<vsg::vec3>(
        camera_handler->get_front());

    const float arrow_x_dot = std::abs(vsg::dot(camera_front, X_AXIS_POSITIVE));
    const float arrow_y_dot = std::abs(vsg::dot(camera_front, Y_AXIS_POSITIVE));
    const float arrow_z_dot = std::abs(vsg::dot(camera_front, Z_AXIS_POSITIVE));

    for (const vsg::Node* node : node_path)
    {
        if (node == arrow_x)
        {
            click_pos = {world_intersection.x, curr_pos.y, curr_pos.z};

            active_arrow = arrow_x;

            active_plain_switch = (arrow_y_dot > arrow_z_dot)
                ? plane_xz_switch
                : plane_xy_switch;

            active_line_switch = line_x_switch;
        }
        else if (node == arrow_y)
        {
            click_pos = {curr_pos.x, world_intersection.y, curr_pos.z};

            active_arrow = arrow_y;

            active_plain_switch = (arrow_x_dot > arrow_z_dot)
                ? plane_yz_switch
                : plane_xy_switch;

            active_line_switch = line_y_switch;
        }
        else if (node == arrow_z)
        {
            click_pos = {curr_pos.x, curr_pos.y, world_intersection.z};

            active_arrow = arrow_z;

            active_plain_switch = (arrow_x_dot > arrow_y_dot)
                ? plane_yz_switch
                : plane_xz_switch;

            active_line_switch = line_z_switch;
        }
        else
        {
            continue;
        }

        prev_intersect_pos = click_pos;
        total_translation = {0.0f, 0.0f, 0.0f};

        active_plain_switch->mask = MASK_CLICKABLE;
        active_line_switch->mask = MASK_GUI1;

        break;
    }

    intersector->intersections.clear();

    return true;
}

void Gizmo::apply(const vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled || !active_arrow)
    {
        return;
    }

    commands.push(new MoveObjectsCommand(selected_objects, total_translation),
        false);

    active_arrow = nullptr;

    active_plain_switch->mask = vsg::MASK_OFF;
    active_plain_switch = nullptr;

    active_line_switch->mask = vsg::MASK_OFF;
    active_line_switch = nullptr;
}

void Gizmo::apply(const vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled || !active_plain_switch)
    {
        return;
    }

    const auto intersector = intersection_handler->apply_(moveEvent);

    this->accept(*intersector);

    const auto intersection = intersection_handler->get_closest_intersection(
        intersector);

    if (!intersection)
    {
        return;
    }

    const auto world_intersection = static_cast<vsg::vec3>(
        intersection->worldIntersection);

    const auto& node_path = intersection->nodePath;
    for (const vsg::Node* node : node_path)
    {
        if (node != active_plain_switch)
        {
            continue;
        }

        vsg::vec3 translation = {0.0f, 0.0f, 0.0f};

        if (active_arrow == arrow_x)
        {
            translation.x = world_intersection.x - prev_intersect_pos.x;
            prev_intersect_pos.x = world_intersection.x;
        }
        else if (active_arrow == arrow_y)
        {
            translation.y = world_intersection.y - prev_intersect_pos.y;
            prev_intersect_pos.y = world_intersection.y;
        }
        else if (active_arrow == arrow_z)
        {
            translation.z = world_intersection.z - prev_intersect_pos.z;
            prev_intersect_pos.z = world_intersection.z;
        }
        else
        {
            continue;
        }

        total_translation += translation;

        for (const auto& object : selected_objects)
        {
            object->move(translation, true);
        }

        return;
    }
}

void Gizmo::update_position()
{
    curr_pos = {0.0f, 0.0f, 0.0f};

    if (settings.gizmo_to_center)
    {
        for (const auto& object : selected_objects)
        {
            const auto& bounds = object->get_bounds();
            curr_pos += (bounds.min + bounds.max) / 2.0f;
        }
    }
    else
    {
        for (const auto& object : selected_objects)
        {
            curr_pos += object->get_translation();
        }
    }

    curr_pos /= static_cast<float>(selected_objects.size());
}

void Gizmo::update_scale()
{
    const auto camera_pos = static_cast<vsg::vec3>(camera_handler->get_eye());

    const auto fov_rad = vsg::radians(static_cast<float>(
        camera_handler->get_fov_deg()));

    const float distance_to_camera = vsg::length(curr_pos - camera_pos);
    const float tan_half_fov = std::tan(fov_rad * 0.5f);
    const float scale = distance_to_camera * tan_half_fov * 0.075f;
    this->matrix = vsg::translate(curr_pos) * vsg::scale(scale);
}
