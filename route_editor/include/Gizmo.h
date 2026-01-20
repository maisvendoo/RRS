#ifndef GIZMO_H
#define GIZMO_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

struct settings_t;

namespace vsg
{

class ButtonReleaseEvent;
class LineSegmentIntersector;
class MoveEvent;
class Node;

}

using MatTrans = vsg::MatrixTransform;
using MatTransPtr = vsg::ref_ptr<MatTrans>;
using GuiSwitch = vsg::Switch;
using GuiSwitchPtr = vsg::ref_ptr<GuiSwitch>;
using SelectedObjectsMap = std::map<MatTransPtr, GuiSwitchPtr>;
using SelectedObjectIterator = SelectedObjectsMap::iterator;
using Intersector = vsg::LineSegmentIntersector;
using IntersectorPtr = vsg::ref_ptr<Intersector>;

class Gizmo : public vsg::Inherit<MatTrans, Gizmo>
{
public:
    Gizmo(
        const settings_t& settings,
        const SelectedObjectsMap& selected_objects
    );

    bool handle_intersections(IntersectorPtr intersector);

    void apply(const vsg::ButtonReleaseEvent& buttonRelease);
    void apply(const vsg::MoveEvent& moveEvent);

    void update();

private:
    const SelectedObjectsMap& selected_objects;
    vsg::ref_ptr<vsg::Node> arrow_x;
    vsg::ref_ptr<vsg::Node> arrow_y;
    vsg::ref_ptr<vsg::Node> arrow_z;
};

#endif // GIZMO_H
