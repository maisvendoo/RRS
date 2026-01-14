#ifndef GIZMO_H
#define GIZMO_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/utils/LineSegmentIntersector.h>

namespace vsg
{

class Node;
class Options;

}

enum class GizmoAxis
{
    NONE,
    X,
    Y,
    Z
};

class Gizmo : public vsg::Inherit<vsg::MatrixTransform, Gizmo>
{
public:
    Gizmo(
        vsg::ref_ptr<vsg::Options>& options,
        float arrow_length,
        float arrow_thickness,
        vsg::vec3 x_axis_color,
        vsg::vec3 y_axis_color,
        vsg::vec3 z_axis_color,
        float opacity
    );

    GizmoAxis handle_intersection(const vsg::LineSegmentIntersector::Intersection& intersection) const;

public:
    vsg::dvec3 translation;

private:
    vsg::ref_ptr<vsg::Node> arrow_x;
    vsg::ref_ptr<vsg::Node> arrow_y;
    vsg::ref_ptr<vsg::Node> arrow_z;
};

#endif // GIZMO_H
