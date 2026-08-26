#ifndef GIZMO_H
#define GIZMO_H

#include "Camera.h"
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
        const vsg::ref_ptr<Camera>& camera,
        CommandList& command_list,
        const vsg::ref_ptr<Mouse>& mouse,
        const VkExtent2D& window_extent
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
    const vsg::ref_ptr<Camera>& camera;
    CommandList& command_list;
    const vsg::ref_ptr<Mouse>& mouse;
    const VkExtent2D& window_extent;

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
