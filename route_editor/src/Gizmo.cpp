#include "Gizmo.h"

#include "Camera.h"
#include "EditorContext.h"
#include "IntersectionHandler.h"
#include "Mask.h"
#include "RouteObject.h"
#include "SingleSwitch.h"
#include "commands/CommandList.h"
#include "commands/TranslateObjects.h"

#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/common.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/ShaderSet.h>

#include <cmath>

static void rotate_geometry_info(
    vsg::GeometryInfo& geometry_info,
    vsg::vec3 direction
)
{
    constexpr vsg::vec3 z_axis = {0.0f, 0.0f, 1.0f};
    if (vsg::length(vsg::cross(z_axis, direction)) > 0.001f)
    {
        const vsg::vec3 axis = vsg::cross(z_axis, direction);
        const float angle = std::acos(vsg::dot(z_axis, direction));
        geometry_info.transform = vsg::rotate(angle, axis);
    }
}

static vsg::ref_ptr<vsg::Node> create_arrow(vsg::Builder& builder,
    vsg::vec3 direction, float length, float thickness, vsg::vec3 color,
    float opacity, vsg::StateInfo& state_info)
{
    vsg::GeometryInfo geometry_info(vsg::box(
        vsg::vec3(-thickness, -thickness, 0.0f),
        vsg::vec3( thickness,  thickness, length)
    ));

    rotate_geometry_info(geometry_info, direction);
}

Gizmo::Gizmo(
    EditorContext& context,
    const gizmo_settings_t& gizmo_settings,
    const vsg::ref_ptr<IntersectionHandler>& intersection_handler,
    const vsg::ref_ptr<Camera>& camera,
    CommandList& command_list
)
    : context_(context)
    , gizmo_settings(gizmo_settings)
    , intersection_handler(intersection_handler)
    , camera(camera)
    , command_list(command_list)
{
    builder_.shaderSet = vsg::createFlatShadedShaderSet();

    const vsg::vec3 arrow_x_color = gizmo_settings.arrow_x_color;
    const vsg::vec3 arrow_y_color = gizmo_settings.arrow_y_color;
    const vsg::vec3 arrow_z_color = gizmo_settings.arrow_z_color;

    const float plane_width = 1.0e6f;
    const float line_thickness = 0.01f;

    vsg::StateInfo state_info;
    state_info.two_sided = true;
    state_info.blending = true;

    const auto create_arrow = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        float thickness = gizmo_settings.arrow_thickness;
        const float length = gizmo_settings.arrow_length;
        const float opacity = gizmo_settings.opacity;

        vsg::box box = {
            vsg::vec3(-thickness, -thickness, 0.0f),
            vsg::vec3( thickness,  thickness, length)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, direction);
        geometry_info.color = {color, opacity};

        const auto cylinder = builder_.createCylinder(geometry_info, state_info);

        thickness *= 3.0f;

        box.min = {-thickness, -thickness, length};
        box.max = { thickness,  thickness, length + thickness * 5.0f};

        geometry_info.set(box);

        const auto cone = builder_.createCone(geometry_info, state_info);

        const auto arrow = vsg::Group::create();
        arrow->addChild(cylinder);
        arrow->addChild(cone);

        return arrow;
    };

    const auto create_plane = [&](vsg::vec3 normal) -> vsg::ref_ptr<vsg::Node>
    {
        const float width = plane_width;

        const vsg::box box = {
            vsg::vec3(-width, -width, 0.0f),
            vsg::vec3( width,  width, 0.0f)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, normal);

        return builder_.createQuad(geometry_info, state_info);
    };

    const auto create_line = [&](vsg::vec3 direction,
        vsg::vec3 color) -> vsg::ref_ptr<vsg::Node>
    {
        const float width = plane_width;
        const float thickness = line_thickness;
        const float opacity = gizmo_settings.opacity;

        const vsg::box box = {
            vsg::vec3(-thickness, -thickness, -width),
            vsg::vec3( thickness,  thickness,  width)
        };

        vsg::GeometryInfo geometry_info(box);
        rotate_geometry_info(geometry_info, direction);
        geometry_info.color = {color, opacity};

        return builder_.createCylinder(geometry_info, state_info);
    };

    arrow_x_ = create_arrow(vsg::vec3(1.0f, 0.0f, 0.0f), arrow_x_color);
    arrow_y_ = create_arrow(vsg::vec3(0.0f, 1.0f, 0.0f), arrow_y_color);
    arrow_z_ = create_arrow(vsg::vec3(0.0f, 0.0f, 1.0f), arrow_z_color);

    const auto plane_yz = create_plane(vsg::vec3(1.0f, 0.0f, 0.0f));
    const auto plane_xz = create_plane(vsg::vec3(0.0f, 1.0f, 0.0f));
    const auto plane_xy = create_plane(vsg::vec3(0.0f, 0.0f, 1.0f));

    plane_yz_switch_ = SingleSwitch::create(vsg::MASK_OFF, plane_yz);
    plane_xz_switch_ = SingleSwitch::create(vsg::MASK_OFF, plane_xz);
    plane_xy_switch_ = SingleSwitch::create(vsg::MASK_OFF, plane_xy);

    const auto line_x = create_line(vsg::vec3(1.0f, 0.0f, 0.0f), arrow_x_color);
    const auto line_y = create_line(vsg::vec3(0.0f, 1.0f, 0.0f), arrow_y_color);
    const auto line_z = create_line(vsg::vec3(0.0f, 0.0f, 1.0f), arrow_z_color);

    line_x_switch_ = SingleSwitch::create(vsg::MASK_OFF, line_x);
    line_y_switch_ = SingleSwitch::create(vsg::MASK_OFF, line_y);
    line_z_switch_ = SingleSwitch::create(vsg::MASK_OFF, line_z);

    matrix_transform_ = vsg::MatrixTransform::create();
    matrix_transform_->addChild(arrow_x_);
    matrix_transform_->addChild(arrow_y_);
    matrix_transform_->addChild(arrow_z_);
    matrix_transform_->addChild(plane_yz_switch_);
    matrix_transform_->addChild(plane_xz_switch_);
    matrix_transform_->addChild(plane_xy_switch_);
    matrix_transform_->addChild(line_x_switch_);
    matrix_transform_->addChild(line_y_switch_);
    matrix_transform_->addChild(line_z_switch_);

    this->node = matrix_transform_;

    update_visibility();
}

bool Gizmo::handle_intersections()
{
    const auto intersector = intersection_handler->get_lmb_intersector();
    if (!intersector)
    {
        return false;
    }

    this->accept(*intersector);

    const auto intersection = intersection_handler->get_closest_intersection(intersector);
    if (!intersection)
    {
        return false;
    }

    const vsg::dvec3& world_intersection = intersection->worldIntersection;
    const vsg::dvec3& camera_front = camera->get_front();

    const double arrow_x_dot = std::abs(vsg::dot(camera_front, vsg::dvec3(1.0, 0.0, 0.0)));
    const double arrow_y_dot = std::abs(vsg::dot(camera_front, vsg::dvec3(0.0, 1.0, 0.0)));
    const double arrow_z_dot = std::abs(vsg::dot(camera_front, vsg::dvec3(0.0, 0.0, 1.0)));

    for (const vsg::Node* const node : intersection->nodePath)
    {
        click_pos_ = curr_pos_;

        if (node == arrow_x_)
        {
            click_pos_.x = world_intersection.x;
            active_arrow_ = arrow_x_;
            active_plain_switch_ = (arrow_y_dot > arrow_z_dot)
                ? plane_xz_switch_
                : plane_xy_switch_;
            active_line_switch_ = line_x_switch_;
        }
        else if (node == arrow_y_)
        {
            click_pos_.y = world_intersection.y;
            active_arrow_ = arrow_y_;
            active_plain_switch_ = (arrow_x_dot > arrow_z_dot)
                ? plane_yz_switch_
                : plane_xy_switch_;
            active_line_switch_ = line_y_switch_;
        }
        else if (node == arrow_z_)
        {
            click_pos_.z = world_intersection.z;
            active_arrow_ = arrow_z_;
            active_plain_switch_ = (arrow_x_dot > arrow_y_dot)
                ? plane_yz_switch_
                : plane_xz_switch_;
            active_line_switch_ = line_z_switch_;
        }
        else
        {
            continue;
        }

        prev_intersect_pos_ = click_pos_;
        total_translation_.set(0.0, 0.0, 0.0);

        active_plain_switch_->mask = MASK_CLICKABLE;
        active_line_switch_->mask = MASK_GUI1;

        for (const auto& object : context_.selected_objects)
        {
            object->save_matrix();
        }

        break;
    }

    intersector->intersections.clear();

    return true;
}

void Gizmo::apply(const vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled || !active_arrow_)
    {
        return;
    }

    command_list.push(new TranslateObjects(
        context_, context_.selected_objects, total_translation_), false);

    active_arrow_ = nullptr;

    active_plain_switch_->mask = vsg::MASK_OFF;
    active_plain_switch_ = nullptr;

    active_line_switch_->mask = vsg::MASK_OFF;
    active_line_switch_ = nullptr;
}

void Gizmo::apply(const vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled || !active_plain_switch_)
    {
        return;
    }

    const auto intersector = intersection_handler->apply_(moveEvent.x, moveEvent.y);

    this->accept(*intersector);

    const auto intersection =
        intersection_handler->get_closest_intersection(intersector);

    if (!intersection)
    {
        return;
    }

    const vsg::dvec3& world_intersection = intersection->worldIntersection;

    for (const vsg::Node* const node : intersection->nodePath)
    {
        if (node != active_plain_switch_)
        {
            continue;
        }

        vsg::dvec3 translation = {0.0, 0.0, 0.0};

        if (active_arrow_ == arrow_x_)
        {
            translation.x = world_intersection.x - prev_intersect_pos_.x;
            prev_intersect_pos_.x = world_intersection.x;
        }
        else if (active_arrow_ == arrow_y_)
        {
            translation.y = world_intersection.y - prev_intersect_pos_.y;
            prev_intersect_pos_.y = world_intersection.y;
        }
        else if (active_arrow_ == arrow_z_)
        {
            translation.z = world_intersection.z - prev_intersect_pos_.z;
            prev_intersect_pos_.z = world_intersection.z;
        }
        else
        {
            continue;
        }

        total_translation_ += translation;

        for (const auto& object : context_.selected_objects)
        {
            object->move(translation);
        }

        return;
    }
}

const vsg::dvec3& Gizmo::get_curr_pos() const
{
    return curr_pos_;
}

void Gizmo::update_visibility()
{
    this->mask = context_.selected_objects.empty()
        ? vsg::MASK_OFF
        : MASK_GUI1 | MASK_CLICKABLE;

    const vsg::dvec3& camera_pos = camera->get_look_at()->eye;
    const double fov_rad = vsg::radians(camera->get_perspective()->fieldOfViewY);

    const double distance_to_camera = vsg::length(curr_pos_ - camera_pos);
    const double tan_half_fov = std::tan(fov_rad * 0.5);
    scale_ = distance_to_camera * tan_half_fov * 0.075;

    matrix_transform_->matrix = vsg::translate(curr_pos_) * vsg::scale(scale_);
}

void Gizmo::update_position()
{
    curr_pos_ = {0.0, 0.0, 0.0};

    if (gizmo_settings.to_center)
    {
        for (const auto& object : context_.selected_objects)
        {
            const vsg::dbox& bounds = object->get_bounds();
            curr_pos_ += (bounds.min + bounds.max) * 0.5;
        }
    }
    else
    {
        for (const auto& object : context_.selected_objects)
        {
            curr_pos_ += object->get_translation();
        }
    }

    curr_pos_ /= context_.selected_objects.size();

    matrix_transform_->matrix = vsg::translate(curr_pos_) * vsg::scale(scale_);
}
