#ifndef GIZMO_H
#define GIZMO_H

#include "SelectedObjects.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/utils/Builder.h>

#include <map>

class CameraHandler;
class IntersectionHandler;
class RouteObject;
class SingleSwitch;
struct settings_t;

namespace vsg
{

class ButtonReleaseEvent;
class MoveEvent;
class Node;

}

class Gizmo : public vsg::Inherit<vsg::MatrixTransform, Gizmo>
{
public:
    Gizmo(
        const settings_t& settings,
        vsg::ref_ptr<CameraHandler> camera_handler,
        vsg::ref_ptr<IntersectionHandler> intersection_handler,
        const SelectedObjects& selected_objects
    );

    bool handle_intersections();

    void apply(const vsg::ButtonReleaseEvent& buttonRelease);
    void apply(const vsg::MoveEvent& moveEvent);

    void update_position();
    void update_scale();

private:
    vsg::ref_ptr<CameraHandler> camera_handler;
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    const SelectedObjects& selected_objects;

    vsg::Builder builder;
    vsg::ref_ptr<vsg::Node> arrow_x;
    vsg::ref_ptr<vsg::Node> arrow_y;
    vsg::ref_ptr<vsg::Node> arrow_z;
    vsg::ref_ptr<SingleSwitch> plane_yz_switch;
    vsg::ref_ptr<SingleSwitch> plane_xz_switch;
    vsg::ref_ptr<SingleSwitch> plane_xy_switch;

    vsg::vec3 curr_position;
    vsg::vec3 click_position;
    vsg::vec3 click_position_offset;
    std::map<vsg::ref_ptr<RouteObject>, vsg::vec3> selected_objects_begin_poss;
    vsg::ref_ptr<vsg::Node> active_arrow;
    vsg::ref_ptr<SingleSwitch> active_plain_switch;
};

#endif // GIZMO_H
