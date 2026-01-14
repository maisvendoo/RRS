#ifndef INTERSECTION_HANDLER_H
#define INTERSECTION_HANDLER_H

#include "Gizmo.h"
#include "ObjectSelector.h"
#include "Outline.h"
#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/utils/LineSegmentIntersector.h>

struct IntersectionHandlerCreateInfo;
struct settings_t;

namespace vsg
{

class ButtonPressEvent;
class Camera;
class FrameEvent;
class Group;
class LookAt;
class MatrixTransform;
class Options;
class Viewer;

}

class IntersectionHandler : public vsg::Inherit<vsg::Visitor, IntersectionHandler>
{
public:
    IntersectionHandler(
        const settings_t& settings,
        vsg::ref_ptr<vsg::Options> options,
        vsg::ref_ptr<vsg::LookAt> look_at,
        vsg::ref_ptr<vsg::Camera> camera,
        vsg::ref_ptr<vsg::Group> scene_group,
        vsg::ref_ptr<vsg::Group> gui_group,
        vsg::observer_ptr<vsg::Viewer> observer_viewer,
        vsg::ref_ptr<ObjectSelector> object_selector
    );

    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;
    void apply(vsg::FrameEvent& frame) override;

public:
    vsg::ref_ptr<vsg::MatrixTransform>* get_curr_matrix_transform_ptr();

private:
    bool handle_gui_intersections(vsg::ref_ptr<vsg::LineSegmentIntersector> intersector);
    void handle_scene_intersections(vsg::ref_ptr<vsg::LineSegmentIntersector> intersector);

private:
    const settings_t& settings;
    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::LookAt> look_at;
    vsg::ref_ptr<vsg::Camera> camera;
    vsg::ref_ptr<vsg::Group> scene_group;
    vsg::ref_ptr<vsg::Group> gui_group;
    vsg::observer_ptr<vsg::Viewer> observer_viewer;

    vsg::ref_ptr<ObjectSelector> object_selector;
    vsg::ref_ptr<vsg::MatrixTransform> curr_matrix_transform;
    vsg::dvec3 center_offset;
    GizmoAxis active_gizmo_axis;
};

#endif // INTERSECTION_HANDLER_H
