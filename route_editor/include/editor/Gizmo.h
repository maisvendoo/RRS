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
    Gizmo(EditorContext& editor_context);

    bool handle_intersections();

    void apply(const vsg::ButtonReleaseEvent& buttonRelease);
    void apply(const vsg::MoveEvent& moveEvent);

    const vsg::dvec3& get_curr_pos() const;

    void update_visibility();
    void update_position();

private:
    EditorContext& editor_context;

    vsg::Builder builder_;
    vsg::ref_ptr<vsg::MatrixTransform> matrix_transform_;
    vsg::ref_ptr<vsg::Node> arrows[3];
    vsg::ref_ptr<SingleSwitch> plane_switches[3];
    vsg::ref_ptr<SingleSwitch> line_switches[3];

    vsg::dvec3 curr_pos_;
    vsg::dvec3 prev_intersect_pos_;
    vsg::dvec3 total_translation_;
    double scale_;

    int active_arrow_index = -1;
    int active_plain_index = -1;
};

#endif // GIZMO_H
