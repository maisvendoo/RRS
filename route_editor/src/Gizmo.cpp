#include "Gizmo.h"

#include "Camera.h"
#include "EditorContext.h"
#include "Mask.h"
#include "RouteObject.h"
#include "SingleSwitch.h"
#include "commands/CommandList.h"
#include "commands/TranslateObjects.h"

#include <cstdlib>
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
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsg/utils/ShaderSet.h>

#include <cmath>

static void rotate_geometry_info(
    vsg::GeometryInfo& geometry_info,
    vsg::vec3 direction
)
{
    constexpr vsg::vec3 Z_AXIS = {0.0f, 0.0f, 1.0f};
    if (vsg::length(vsg::cross(Z_AXIS, direction)) > 0.001f)
    {
        const vsg::vec3 axis = vsg::cross(Z_AXIS, direction);
        const float angle = std::acos(vsg::dot(Z_AXIS, direction));
        geometry_info.transform = vsg::rotate(angle, axis);
    }
}

Gizmo::Gizmo(
    EditorContext& context,
    const gizmo_settings_t& gizmo_settings,
    const vsg::ref_ptr<Camera>& camera,
    CommandList& command_list
)
    : context_(context)
    , gizmo_settings(gizmo_settings)
    , camera(camera)
    , command_list(command_list)
{
    builder_.shaderSet = vsg::createFlatShadedShaderSet();

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

    matrix_transform_ = vsg::MatrixTransform::create();

    constexpr vsg::vec3 arrow_directions[3] = {
        vsg::vec3(1.0f, 0.0f, 0.0f),
        vsg::vec3(0.0f, 1.0f, 0.0f),
        vsg::vec3(0.0f, 0.0f, 1.0f)
    };

    const vsg::vec3 arrow_colors[3] = {
        gizmo_settings.arrow_x_color,
        gizmo_settings.arrow_y_color,
        gizmo_settings.arrow_z_color
    };

    for (int i = 0; i < 3; ++i)
    {
        arrows[i] = create_arrow(arrow_directions[i], arrow_colors[i]);
        plane_switches[i] = SingleSwitch::create(vsg::MASK_OFF,
            create_plane(arrow_directions[i]));
        line_switches[i] = SingleSwitch::create(vsg::MASK_OFF, create_line(
            arrow_directions[i], arrow_colors[i]));
        matrix_transform_->addChild(arrows[i]);
        matrix_transform_->addChild(plane_switches[i]);
        matrix_transform_->addChild(line_switches[i]);
    }

    this->node = matrix_transform_;

    update_visibility();
}

bool Gizmo::handle_intersections(
    const vsg::ref_ptr<vsg::LineSegmentIntersector>& intersector
)
{
    this->accept(*intersector);

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        return false;
    }

    std::sort(intersections.begin(), intersections.end(),
        [](const auto& lhs, const auto& rhs) -> bool {
            return (lhs->ratio) < (rhs->ratio);
        }
    );

    const auto& intersection = intersections.front();

    constexpr vsg::dvec3 arrow_directions[3] = {
        vsg::dvec3(1.0, 0.0, 0.0),
        vsg::dvec3(0.0, 1.0, 0.0),
        vsg::dvec3(0.0, 0.0, 1.0)
    };

    double arrow_dots[3];
    for (int i = 0; i < 3; ++i)
    {
        arrow_dots[i] = std::abs(vsg::dot(camera->get_front(), arrow_directions[i]));
    }

    for (const vsg::Node* const node : intersection->nodePath)
    {
        if (node == arrows[0])
        {
            active_arrow_index = 0;
            active_plain_index = (arrow_dots[1] > arrow_dots[2]) ? 1 : 2;
        }
        else if (node == arrows[1])
        {
            active_arrow_index = 1;
            active_plain_index = (arrow_dots[2] > arrow_dots[0]) ? 2 : 0;
        }
        else if (node == arrows[2])
        {
            active_arrow_index = 2;
            active_plain_index = (arrow_dots[0] > arrow_dots[1]) ? 0 : 1;
        }
        else
        {
            continue;
        }

        vsg::dvec3 click_pos = curr_pos_;
        click_pos[active_arrow_index] =
            intersection->worldIntersection[active_arrow_index];

        prev_intersect_pos_ = click_pos;
        total_translation_.set(0.0, 0.0, 0.0);

        plane_switches[active_plain_index]->mask = MASK_CLICKABLE;
        line_switches[active_arrow_index]->mask = MASK_GUI1;

        for (const auto& object : context_.selected_objects)
        {
            object->save_matrix();
        }

        break;
    }

    intersections.clear();

    return true;
}

void Gizmo::apply(const vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled || active_arrow_index < 0)
    {
        return;
    }

    command_list.push(new TranslateObjects(
        context_, context_.selected_objects, total_translation_), false);

    plane_switches[active_plain_index]->mask = vsg::MASK_OFF;
    line_switches[active_arrow_index]->mask = vsg::MASK_OFF;

    active_arrow_index = -1;
    active_plain_index = -1;
}

void Gizmo::apply(const vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled || active_plain_index < 0)
    {
        return;
    }

    const auto intersector = vsg::LineSegmentIntersector::create(*camera,
        moveEvent.x, moveEvent.y);
    if (!intersector)
    {
        return;
    }
    intersector->traversalMask = MASK_CLICKABLE;

    this->accept(*intersector);

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        return;
    }

    std::sort(intersections.begin(), intersections.end(),
        [](const auto& lhs, const auto& rhs) -> bool {
            return (lhs->ratio) < (rhs->ratio);
        }
    );

    const auto& intersection = intersections.front();

    const vsg::dvec3& world_intersection = intersection->worldIntersection;

    for (const vsg::Node* const node : intersection->nodePath)
    {
        if ((node != plane_switches[active_plain_index]) || (active_arrow_index < 0))
        {
            continue;
        }

        vsg::dvec3 translation = {0.0, 0.0, 0.0};

        translation[active_arrow_index] =
            world_intersection[active_arrow_index] -
            prev_intersect_pos_[active_arrow_index];

        prev_intersect_pos_[active_arrow_index] =
            world_intersection[active_arrow_index];

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

static vsg::dvec3 calculate_position_pivot(
    const RouteObjects& selected_objects
)
{
    vsg::dvec3 pos = {0.0, 0.0, 0.0};
    for (const auto& object : selected_objects)
    {
        pos += object->get_translation();
    }
    pos /= selected_objects.size();
    return pos;
}

static vsg::dvec3 calculate_position_center(
    const RouteObjects& selected_objects
)
{
    vsg::dvec3 pos = {0.0, 0.0, 0.0};
    for (const auto& object : selected_objects)
    {
        const vsg::dbox& bounds = object->get_bounds();
        pos += (bounds.min + bounds.max) * 0.5;
    }
    pos /= selected_objects.size();
    return pos;
}

void Gizmo::update_position()
{
    curr_pos_ = gizmo_settings.to_center
        ? calculate_position_center(context_.selected_objects)
        : calculate_position_pivot(context_.selected_objects);

    matrix_transform_->matrix = vsg::translate(curr_pos_) * vsg::scale(scale_);
}
