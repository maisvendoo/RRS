#ifndef GIZMO_H
#define GIZMO_H

#include "SingleSwitch.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/utils/Builder.h>

struct EditorContext;

namespace vsg
{

class ButtonReleaseEvent;
class MatrixTransform;
class MoveEvent;
class Node;

}

class Gizmo : public vsg::Inherit<SingleSwitch, Gizmo>
{
public:
    Gizmo(EditorContext& context);

    bool handle_intersections();

    void apply(const vsg::ButtonReleaseEvent& buttonRelease);
    void apply(const vsg::MoveEvent& moveEvent);

    const vsg::dvec3& get_curr_pos() const;

    void update_visibility();
    void update_position();

private:
    EditorContext& context;

    vsg::Builder builder;
    vsg::ref_ptr<vsg::MatrixTransform> matrix_transform;
    vsg::ref_ptr<vsg::Node> arrow_x;
    vsg::ref_ptr<vsg::Node> arrow_y;
    vsg::ref_ptr<vsg::Node> arrow_z;
    vsg::ref_ptr<SingleSwitch> plane_yz_switch;
    vsg::ref_ptr<SingleSwitch> plane_xz_switch;
    vsg::ref_ptr<SingleSwitch> plane_xy_switch;
    vsg::ref_ptr<SingleSwitch> line_x_switch;
    vsg::ref_ptr<SingleSwitch> line_y_switch;
    vsg::ref_ptr<SingleSwitch> line_z_switch;

    vsg::dvec3 curr_pos;
    vsg::dvec3 click_pos;
    vsg::dvec3 prev_intersect_pos;
    vsg::dvec3 total_translation;
    double scale;

    vsg::ref_ptr<vsg::Node> active_arrow;
    vsg::ref_ptr<SingleSwitch> active_plain_switch;
    vsg::ref_ptr<SingleSwitch> active_line_switch;
};

#endif // GIZMO_H
