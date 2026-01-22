#include "Gizmo.h"

#include "IntersectionHandler.h"
#include "Mask.h"
#include "SelectedObjectsMap.h"
#include "Settings.h"

#include <vsg/app/ViewMatrix.h>
#include <vsg/core/Mask.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Switch.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/ComputeBounds.h>
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsg/utils/ShaderSet.h>

#include <cassert>
#include <cmath>

enum
{
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    TOTAL_AXES
};

Gizmo::Gizmo(
    const settings_t& settings,
    vsg::ref_ptr<vsg::LookAt> look_at,
    const SelectedObjectsMap& selected_objects
)
    : settings(settings)
    , look_at(look_at)
    , selected_objects(selected_objects)
{
    builder.shaderSet = vsg::createFlatShadedShaderSet();

    vsg::ref_ptr<vsg::Node>* arrows[TOTAL_AXES];
    arrows[AXIS_X] = &arrow_x;
    arrows[AXIS_Y] = &arrow_y;
    arrows[AXIS_Z] = &arrow_z;

    vsg::vec3 arrow_directions[TOTAL_AXES];
    arrow_directions[AXIS_X] = {1.0f, 0.0f, 0.0f};
    arrow_directions[AXIS_Y] = {0.0f, 1.0f, 0.0f};
    arrow_directions[AXIS_Z] = {0.0f, 0.0f, 1.0f};

    vsg::vec3 arrow_colors[TOTAL_AXES];
    arrow_colors[AXIS_X] = settings.gizmo_arrow_x_color;
    arrow_colors[AXIS_Y] = settings.gizmo_arrow_y_color;
    arrow_colors[AXIS_Z] = settings.gizmo_arrow_z_color;

    vsg::ref_ptr<vsg::Node>* planes[TOTAL_AXES];
    planes[AXIS_X] = &plane_yz;
    planes[AXIS_Y] = &plane_xz;
    planes[AXIS_Z] = &plane_xy;

    vsg::Mask** plane_masks[TOTAL_AXES];
    plane_masks[AXIS_X] = &plane_mask_yz;
    plane_masks[AXIS_Y] = &plane_mask_xz;
    plane_masks[AXIS_Z] = &plane_mask_xy;

    const float plane_size = 100.0f;
    const float line_size = 0.01f;

    const auto switch_group = vsg::Switch::create();
    switch_group->children.reserve(2 * TOTAL_AXES);

    for (int i = 0; i < TOTAL_AXES; ++i)
    {
        *arrows[i] = create_arrow(arrow_directions[i], arrow_colors[i]);
        this->addChild(*arrows[i]);

        *planes[i] = create_plane(plane_size, i);
        switch_group->addChild(vsg::MASK_OFF, *planes[i]);
        *plane_masks[i] = &switch_group->children.back().mask;

        switch_group->addChild(vsg::Mask{MASK_GUI}, create_line(
            plane_size, line_size, i, arrow_colors[i]));
    }

    this->addChild(switch_group);
}

bool Gizmo::handle_intersections(
    vsg::ref_ptr<vsg::LineSegmentIntersector> intersector
)
{
    this->accept(*intersector);

    auto& intersections = intersector->intersections;
    if (intersections.empty())
    {
        return false;
    }

    IntersectionHandler::sort_intersections(intersections);

    vsg::ref_ptr<vsg::Node> clicked_arrow;
    int index = -1;

    vsg::ref_ptr<vsg::Node> arrows[TOTAL_AXES];
    arrows[AXIS_X] = arrow_x;
    arrows[AXIS_Y] = arrow_y;
    arrows[AXIS_Z] = arrow_z;

    vsg::vec3 arrow_directions[TOTAL_AXES];
    arrow_directions[AXIS_X] = {1.0f, 0.0f, 0.0f};
    arrow_directions[AXIS_Y] = {0.0f, 1.0f, 0.0f};
    arrow_directions[AXIS_Z] = {0.0f, 0.0f, 1.0f};

    vsg::ref_ptr<vsg::Node> planes[TOTAL_AXES];
    planes[AXIS_X] = plane_yz;
    planes[AXIS_Y] = plane_xz;
    planes[AXIS_Z] = plane_xy;

    vsg::Mask* plane_masks[TOTAL_AXES];
    plane_masks[AXIS_X] = plane_mask_yz;
    plane_masks[AXIS_Y] = plane_mask_xz;
    plane_masks[AXIS_Z] = plane_mask_xy;

    const auto& node_path = intersections.front()->nodePath;
    assert(!node_path.empty());

    for (const vsg::Node* const node : node_path)
    {
        for (int i = 0; i < TOTAL_AXES; ++i)
        {
            if (node == arrows[i])
            {
                clicked_arrow = arrows[i];
                index = i;

                break;
            }
        }

        if (clicked_arrow)
        {
            break;
        }
    }

    if (!clicked_arrow)
    {
        intersector->intersections.clear();

        return false;
    }

    const auto world_intersection = static_cast<vsg::vec3>(
        intersections.front()->worldIntersection);

    begin_position = position;
    begin_position[index] = world_intersection[index];

    selected_objects_begin_matrixes.clear();
    for (const auto& [object, _] : selected_objects)
    {
        selected_objects_begin_matrixes[object] = object->matrix;
    }

    const auto camera_pos = static_cast<vsg::vec3>(look_at->eye);
    const auto camera_to_gizmo = vsg::normalize(begin_position - camera_pos);

    float max_dot = -1.0f;
    int max_index = -1;

    for (int i = 0; i < TOTAL_AXES; ++i)
    {
        if (i == index)
        {
            continue;
        }

        const float dot = std::abs(vsg::dot(camera_to_gizmo,
            arrow_directions[i]));

        if (dot > max_dot)
        {
            max_dot = dot;
            max_index = i;
        }
    }

    active_plain = planes[max_index];
    active_plain_mask = plane_masks[max_index];
    *active_plain_mask = MASK_CLICKABLE;

    intersector->intersections.clear();

    return true;
}

void Gizmo::apply(const vsg::ButtonReleaseEvent& buttonRelease)
{
    (void)buttonRelease;
}

void Gizmo::apply(const vsg::MoveEvent& moveEvent)
{
    (void)moveEvent;
}

void Gizmo::update_position()
{
    position = {0.0f, 0.0f, 0.0f};

    for (const auto& [object, _] : selected_objects)
    {
        vsg::ComputeBounds compute_bounds;
        compute_bounds.useNodeBounds = false;

        object->accept(compute_bounds);
        const auto& bounds = compute_bounds.bounds;

        position += static_cast<vsg::vec3>((bounds.min + bounds.max) / 2.0);
    }

    position /= static_cast<float>(selected_objects.size());

    this->matrix = vsg::translate(position);
}

void Gizmo::update_scale()
{
    const auto camera_pos = static_cast<vsg::vec3>(look_at->eye);
    const float distance_to_camera = vsg::length(position - camera_pos);
    const float scale = 0.075f * distance_to_camera;
    this->matrix = vsg::translate(position) * vsg::scale(scale);
}

vsg::ref_ptr<vsg::Node> Gizmo::create_arrow(
    const vsg::vec3& direction,
    const vsg::vec3& color
)
{
    const float thickness = settings.gizmo_arrow_thickness;
    const float length = settings.gizmo_arrow_length;
    const float opacity = settings.gizmo_opacity;

    vsg::box box = {
        vsg::vec3{-0.5f * thickness, -0.5f * thickness, 0.0f},
        vsg::vec3{ 0.5f * thickness,  0.5f * thickness, length}
    };

    vsg::GeometryInfo geometry_info(box);
    geometry_info.color = {color, opacity};

    constexpr vsg::vec3 Z_AXIS_POSITIVE = {0.0f, 0.0f, 1.0f};

    if (vsg::length(vsg::cross(Z_AXIS_POSITIVE, direction)) > 0.001f)
    {
        const vsg::vec3 axis = vsg::cross(Z_AXIS_POSITIVE, direction);
        const float angle = std::acos(vsg::dot(Z_AXIS_POSITIVE, direction));
        geometry_info.transform = vsg::rotate(angle, axis);
    }

    vsg::StateInfo state_info;
    state_info.blending = true;

    const auto cylinder = builder.createCylinder(geometry_info, state_info);

    box.min = {-1.5f * thickness, -1.5f * thickness, length};
    box.max = { 1.5f * thickness,  1.5f * thickness, length + 7.0f * thickness};
    geometry_info.set(box);

    const auto cone = builder.createCone(geometry_info, state_info);

    const auto arrow = vsg::Group::create();
    arrow->addChild(cylinder);
    arrow->addChild(cone);

    return arrow;
}

vsg::ref_ptr<vsg::Node> Gizmo::create_plane(
    const float plane_size,
    const int zero_component_index
)
{
    vsg::vec3 min_vec = {0.0f, 0.0f, 0.0f};
    vsg::vec3 max_vec = {0.0f, 0.0f, 0.0f};

    for (int i = 0; i < 3; ++i)
    {
        if (i != zero_component_index)
        {
            min_vec[i] = -plane_size;
            max_vec[i] = plane_size;
        }
    }

    const vsg::GeometryInfo geometry_info(vsg::box(min_vec, max_vec));

    return builder.createQuad(geometry_info);
}

vsg::ref_ptr<vsg::Node> Gizmo::create_line(
    const float plane_size,
    const float line_size,
    const int plane_component_index,
    const vsg::vec3& color
)
{
    vsg::vec3 min_vec = {0.0f, 0.0f, 0.0f};
    vsg::vec3 max_vec = {0.0f, 0.0f, 0.0f};

    for (int i = 0; i < 3; ++i)
    {
        if (i == plane_component_index)
        {
            min_vec[i] = -plane_size;
            max_vec[i] = plane_size;
        }
        else
        {
            min_vec[i] = -line_size;
            max_vec[i] = line_size;
        }
    }

    vsg::GeometryInfo geometry_info(vsg::box(min_vec, max_vec));
    geometry_info.color = {color, settings.gizmo_opacity};

    vsg::StateInfo state_info;
    state_info.blending = true;

    return builder.createCylinder(geometry_info, state_info);
}
