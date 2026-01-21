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

static vsg::ref_ptr<vsg::Node> create_arrow(
    const settings_t& settings,
    vsg::Builder& builder,
    const vsg::vec3& direction,
    const vsg::vec3& color
);

Gizmo::Gizmo(
    const settings_t& settings,
    vsg::ref_ptr<vsg::LookAt> look_at,
    const SelectedObjectsMap& selected_objects
)
    : look_at(look_at)
    , selected_objects(selected_objects)
{
    vsg::Builder builder;
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

    for (int i = 0; i < TOTAL_ARROWS; ++i)
    {
        *arrows[i] = create_arrow(settings, builder,
            arrow_directions[i], arrow_colors[i]);

        this->addChild(*arrows[i]);
    }

    const float quad_width = 100.0f;
    const float line_size = 0.01f;

    vsg::GeometryInfo geometry_info;

    geometry_info.set(vsg::box(vsg::vec3(0.0f, -quad_width, -quad_width),
        vsg::vec3(0.0f, quad_width, quad_width)));

    plane_yz = builder.createQuad(geometry_info);

    geometry_info.set(vsg::box(vsg::vec3(-quad_width, 0.0f, -quad_width),
        vsg::vec3(quad_width, 0.0f, quad_width)));

    plane_xz = builder.createQuad(geometry_info);

    geometry_info.set(vsg::box(vsg::vec3(-quad_width, -quad_width, 0.0f),
        vsg::vec3(quad_width, quad_width, 0.0f)));

    plane_xy = builder.createQuad(geometry_info);

    const auto planes_switch = vsg::Switch::create();
    planes_switch->addChild(vsg::Mask{MASK_INVISIBLE}, plane_yz);
    planes_switch->addChild(vsg::Mask{MASK_INVISIBLE}, plane_xz);
    planes_switch->addChild(vsg::Mask{MASK_INVISIBLE}, plane_xy);

    this->addChild(planes_switch);

    vsg::StateInfo state_info;
    state_info.blending = true;

    geometry_info.set(vsg::box(vsg::vec3(-quad_width, -line_size, -line_size),
        vsg::vec3(quad_width, line_size, line_size)));

    geometry_info.color = {arrow_colors[ARROW_X], settings.gizmo_opacity};

    const auto line_x = builder.createCylinder(geometry_info, state_info);

    const auto line_x_switch = vsg::Switch::create();
    line_x_switch->addChild(vsg::MASK_ALL, line_x);

    line_x_mask = &line_x_switch->children[0].mask;
    assert(line_x_mask);

    this->addChild(line_x_switch);

    geometry_info.set(vsg::box(vsg::vec3(-line_size, -quad_width, -line_size),
        vsg::vec3(line_size, quad_width, line_size)));

    geometry_info.color = {arrow_colors[ARROW_Y], settings.gizmo_opacity};

    const auto line_y = builder.createCylinder(geometry_info, state_info);

    const auto line_y_switch = vsg::Switch::create();
    line_y_switch->addChild(vsg::MASK_ALL, line_y);

    line_y_mask = &line_y_switch->children[0].mask;
    assert(line_y_mask);

    this->addChild(line_y_switch);

    geometry_info.set(vsg::box(vsg::vec3(-line_size, -line_size, -quad_width),
        vsg::vec3(line_size, line_size, quad_width)));

    geometry_info.color = {arrow_colors[ARROW_Z], settings.gizmo_opacity};

    const auto line_z = builder.createCylinder(geometry_info, state_info);

    const auto line_z_switch = vsg::Switch::create();
    line_z_switch->addChild(vsg::MASK_ALL, line_z);

    line_z_mask = &line_z_switch->children[0].mask;
    assert(line_z_mask);

    this->addChild(line_z_switch);
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

vsg::ref_ptr<vsg::Node> create_arrow(
    const settings_t& settings,
    vsg::Builder& builder,
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
