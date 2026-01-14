#include "Gizmo.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/maths/box.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/LineSegmentIntersector.h>
#include <vsg/utils/ShaderSet.h>

#include <cmath>

static vsg::ref_ptr<vsg::Node> create_arrow(
    vsg::Builder& builder,
    const vsg::vec3& direction,
    float length,
    float thickness,
    const vsg::vec3& color,
    float opacity
);

Gizmo::Gizmo(
    vsg::ref_ptr<vsg::Options>& options,
    float arrow_length,
    float arrow_thickness,
    vsg::vec3 x_axis_color,
    vsg::vec3 y_axis_color,
    vsg::vec3 z_axis_color,
    float opacity
)
{
    vsg::Builder builder;
    // builder.options = options;
    // builder.sharedObjects = options->sharedObjects;
    builder.shaderSet = vsg::createFlatShadedShaderSet();

    arrow_x = create_arrow(builder, vsg::vec3(1.0f, 0.0f, 0.0f),
        arrow_length, arrow_thickness, x_axis_color, opacity);

    arrow_y = create_arrow(builder, vsg::vec3(0.0f, 1.0f, 0.0f),
        arrow_length, arrow_thickness, y_axis_color, opacity);

    arrow_z = create_arrow(builder, vsg::vec3(0.0f, 0.0f, 1.0f),
        arrow_length, arrow_thickness, z_axis_color, opacity);

    this->children = {arrow_x, arrow_y, arrow_z};
}

GizmoAxis Gizmo::handle_intersection(const vsg::LineSegmentIntersector::Intersection& intersection) const
{
    for (const vsg::Node* node : intersection.nodePath)
    {
        if (node == arrow_x)
        {
            return GizmoAxis::X;
        }
        else if (node == arrow_y)
        {
            return GizmoAxis::Y;
        }
        else if (node == arrow_z)
        {
            return GizmoAxis::Z;
        }
    }

    return GizmoAxis::NONE;
}

static vsg::ref_ptr<vsg::Node> create_arrow(
    vsg::Builder& builder,
    const vsg::vec3& direction,
    float length,
    float thickness,
    const vsg::vec3& color,
    float opacity
)
{
    vsg::box box = {
        vsg::vec3{-0.5f * thickness, -0.5f * thickness, 0.0},
        vsg::vec3{ 0.5f * thickness,  0.5f * thickness, length}
    };

    vsg::GeometryInfo geometry_info(box);
    geometry_info.color.set(color.x, color.y, color.z, opacity);

    constexpr vsg::vec3 Z_AXIS_POSITIVE = {0.0f, 0.0f, 1.0f};

    if (vsg::length(vsg::cross(Z_AXIS_POSITIVE, direction)) > 0.001f)
    {
        const vsg::vec3 axis = vsg::cross(Z_AXIS_POSITIVE, direction);
        const float angle = std::acos(vsg::dot(Z_AXIS_POSITIVE, direction));
        geometry_info.transform = vsg::rotate(angle, axis);
    }

    vsg::StateInfo state_info;
    state_info.blending = true;

    const vsg::ref_ptr<vsg::Node> cylinder = builder.createCylinder(geometry_info, state_info);

    box.min = vsg::vec3(-1.5f * thickness, -1.5f * thickness, length);
    box.max = vsg::vec3( 1.5f * thickness,  1.5f * thickness, length + 7.0f * thickness);
    geometry_info.set(box);

    const vsg::ref_ptr<vsg::Node> cone = builder.createCone(geometry_info, state_info);

    const vsg::ref_ptr<vsg::Group> arrow = vsg::Group::create();
    arrow->addChild(cylinder);
    arrow->addChild(cone);

    return arrow;
}
