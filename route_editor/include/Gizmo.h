#ifndef GIZMO_H
#define GIZMO_H

#include "SelectedObjectsMap.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/utils/Builder.h>

#include <map>

class CameraHandler;
class SingleSwitch;
struct settings_t;

namespace vsg
{

class ButtonReleaseEvent;
class LineSegmentIntersector;
class MoveEvent;
class Node;

}

class Gizmo : public vsg::Inherit<vsg::MatrixTransform, Gizmo>
{
public:
    Gizmo(
        const settings_t& settings,
        vsg::ref_ptr<CameraHandler> camera_handler,
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
    vsg::ref_ptr<vsg::Node> create_arrow(
        vsg::vec3 direction,
        vsg::vec3 color
    );

    vsg::ref_ptr<vsg::Node> create_plane(
        float plane_size,
        vsg::vec3 rotate_axis,
        vsg::vec3 color,
        float opacity
    );

    vsg::ref_ptr<vsg::Node> create_line(
        const float plane_size,
        const float line_size,
        const int plane_component_index,
        const vsg::vec3& color
    );

private:
    const settings_t& settings;
    vsg::ref_ptr<CameraHandler> camera_handler;
    const SelectedObjectsMap& selected_objects;
    vsg::vec3 position;
    vsg::Builder builder;

    vsg::ref_ptr<vsg::Node> arrow_x;
    vsg::ref_ptr<vsg::Node> arrow_y;
    vsg::ref_ptr<vsg::Node> arrow_z;
    vsg::ref_ptr<vsg::Node> plane_yz;
    vsg::ref_ptr<vsg::Node> plane_xz;
    vsg::ref_ptr<vsg::Node> plane_xy;
    vsg::ref_ptr<SingleSwitch> plane_yz_switch;
    vsg::ref_ptr<SingleSwitch> plane_xz_switch;
    vsg::ref_ptr<SingleSwitch> plane_xy_switch;

    using MatrixTransformPtr = vsg::ref_ptr<vsg::MatrixTransform>;
    using MatrixMap = std::map<MatrixTransformPtr, vsg::dmat4>;

    vsg::vec3 begin_position;
    MatrixMap selected_objects_begin_matrixes;
    vsg::ref_ptr<vsg::Node> active_plain;
    vsg::ref_ptr<SingleSwitch> active_plain_switch;
};

#endif // GIZMO_H
