#ifndef GIZMO_H
#define GIZMO_H

#include "SelectedObjectsMap.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

struct settings_t;

namespace vsg
{

class ButtonReleaseEvent;
class LineSegmentIntersector;
class LookAt;
class MoveEvent;
class Node;

}

class Gizmo : public vsg::Inherit<vsg::MatrixTransform, Gizmo>
{
public:
    Gizmo(
        const settings_t& settings,
        vsg::ref_ptr<vsg::LookAt> look_at,
        const SelectedObjectsMap& selected_objects
    );

    bool handle_intersections(
        vsg::ref_ptr<vsg::LineSegmentIntersector> intersector
    );

    void apply(const vsg::ButtonReleaseEvent& buttonRelease);
    void apply(const vsg::MoveEvent& moveEvent);

    void update_position();
    void update_scale();

private:
    vsg::ref_ptr<vsg::LookAt> look_at;
    const SelectedObjectsMap& selected_objects;
    vsg::ref_ptr<vsg::Node> arrow_x;
    vsg::ref_ptr<vsg::Node> arrow_y;
    vsg::ref_ptr<vsg::Node> arrow_z;
    vsg::vec3 position;
};

#endif // GIZMO_H
