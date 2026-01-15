#include "Gizmo.h"

#include "Settings.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/box.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/Node.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/ShaderSet.h>

#include <cmath>

static vsg::ref_ptr<vsg::Node> create_arrow(
    const settings_t& settings,
    vsg::Builder& builder,
    const vsg::vec3& direction,
    const vsg::vec3& color
);

Gizmo::Gizmo(const settings_t& settings)
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
}

void Gizmo::set_outer_matrix(vsg::dmat4* outer_matrix)
{
    this->outer_matrix = outer_matrix;
}

static vsg::ref_ptr<vsg::Node> create_arrow(
    const settings_t& settings,
    vsg::Builder& builder,
    const vsg::vec3& direction,
    const vsg::vec3& color
)
{
    const float thickness = static_cast<float>(settings.gizmo_arrow_thickness);
    const float length = static_cast<float>(settings.gizmo_arrow_length);
    const float opacity = static_cast<float>(settings.gizmo_opacity);

    vsg::box box = {
        vsg::vec3{-0.5f * thickness, -0.5f * thickness, 0.0f},
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
