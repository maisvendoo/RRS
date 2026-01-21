#include "Gizmo.h"

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

    enum
    {
        ARROW_X,
        ARROW_Y,
        ARROW_Z,
        TOTAL_ARROWS
    };

    vsg::ref_ptr<vsg::Node>* arrows[TOTAL_ARROWS];
    arrows[ARROW_X] = &arrow_x;
    arrows[ARROW_Y] = &arrow_y;
    arrows[ARROW_Z] = &arrow_z;

    vsg::vec3 arrow_directions[TOTAL_ARROWS];
    arrow_directions[ARROW_X] = {1.0f, 0.0f, 0.0f};
    arrow_directions[ARROW_Y] = {0.0f, 1.0f, 0.0f};
    arrow_directions[ARROW_Z] = {0.0f, 0.0f, 1.0f};

    vsg::vec3 arrow_colors[TOTAL_ARROWS];
    arrow_colors[ARROW_X] = settings.gizmo_arrow_x_color;
    arrow_colors[ARROW_Y] = settings.gizmo_arrow_y_color;
    arrow_colors[ARROW_Z] = settings.gizmo_arrow_z_color;

    vsg::ref_ptr<vsg::Node>* planes[TOTAL_ARROWS];
    planes[ARROW_X] = &plane_yz;
    planes[ARROW_Y] = &plane_xz;
    planes[ARROW_Z] = &plane_xy;

    vsg::Mask** line_masks[TOTAL_ARROWS];
    line_masks[ARROW_X] = &line_x_mask;
    line_masks[ARROW_Y] = &line_y_mask;
    line_masks[ARROW_Z] = &line_z_mask;

    const float plane_size = 100.0f;
    const float line_size = 0.01f;

    const auto planes_switch = vsg::Switch::create();

    for (int i = 0; i < TOTAL_ARROWS; ++i)
    {
        *arrows[i] = create_arrow(arrow_directions[i], arrow_colors[i]);
        this->addChild(*arrows[i]);

        *planes[i] = create_plane(plane_size, i);
        planes_switch->addChild(vsg::Mask{MASK_INVISIBLE}, *planes[i]);

        add_line(plane_size, line_size, i, arrow_colors[i], line_masks[i]);
    }

    this->addChild(planes_switch);
}

bool Gizmo::handle_intersections(
    vsg::ref_ptr<vsg::LineSegmentIntersector> intersector
)
{
    intersector->intersections.clear();

    return false;
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

void Gizmo::add_line(
    const float plane_size,
    const float line_size,
    const int plane_component_index,
    const vsg::vec3& color,
    vsg::Mask** line_mask
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

    const auto line = builder.createCylinder(geometry_info, state_info);

    const auto line_switch = vsg::Switch::create();
    line_switch->addChild(vsg::MASK_ALL, line);

    *line_mask = &line_switch->children[0].mask;
    assert(line_mask);

    this->addChild(line_switch);
}
