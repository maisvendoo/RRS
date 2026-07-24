#ifndef GIZMO_H
#define GIZMO_H

#include "Camera.h"
#include "IntersectionHandler.h"
#include "SingleSwitch.h"
#include "settings/GizmoSettings.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/utils/Builder.h>

class CommandList;
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
    Gizmo(
        EditorContext& context,
        const gizmo_settings_t& gizmo_settings,
        const vsg::ref_ptr<IntersectionHandler>& intersection_handler,
        const vsg::ref_ptr<Camera>& camera,
        CommandList& command_list
    );

    bool handle_intersections();

    void apply(const vsg::ButtonReleaseEvent& buttonRelease);
    void apply(const vsg::MoveEvent& moveEvent);

    const vsg::dvec3& get_curr_pos() const;

    void update_visibility();
    void update_position();

private:
    EditorContext& context_;
    const gizmo_settings_t& gizmo_settings;
    const vsg::ref_ptr<IntersectionHandler>& intersection_handler;
    const vsg::ref_ptr<Camera>& camera;
    CommandList& command_list;

    vsg::Builder builder_;
    vsg::ref_ptr<vsg::MatrixTransform> matrix_transform_;
    vsg::ref_ptr<vsg::Node> arrow_x_;
    vsg::ref_ptr<vsg::Node> arrow_y_;
    vsg::ref_ptr<vsg::Node> arrow_z_;
    vsg::ref_ptr<SingleSwitch> plane_yz_switch_;
    vsg::ref_ptr<SingleSwitch> plane_xz_switch_;
    vsg::ref_ptr<SingleSwitch> plane_xy_switch_;
    vsg::ref_ptr<SingleSwitch> line_x_switch_;
    vsg::ref_ptr<SingleSwitch> line_y_switch_;
    vsg::ref_ptr<SingleSwitch> line_z_switch_;

    vsg::dvec3 curr_pos_;
    vsg::dvec3 click_pos_;
    vsg::dvec3 prev_intersect_pos_;
    vsg::dvec3 total_translation_;
    double scale_;

    vsg::ref_ptr<vsg::Node> active_arrow_;
    vsg::ref_ptr<SingleSwitch> active_plain_switch_;
    vsg::ref_ptr<SingleSwitch> active_line_switch_;
};

#endif // GIZMO_H
