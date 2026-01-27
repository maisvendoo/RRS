#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include "SelectedObjectsMap.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>

class CameraHandler;
class Gizmo;
class IntersectionHandler;
class KeyboardHandler;
class SceneGraph;
class SingleSwitch;
class SwitchGroup;
struct settings_t;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class FrameEvent;
class MatrixTransform;
class MoveEvent;
class PagedLOD;
class Viewer;

}

class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    ObjectSelector(
        const settings_t& settings,
        vsg::ref_ptr<KeyboardHandler> keyboard_handler,
        vsg::ref_ptr<CameraHandler> camera_handler,
        vsg::ref_ptr<IntersectionHandler> intersection_handler,
        vsg::ref_ptr<SceneGraph> scene_graph,
        vsg::observer_ptr<vsg::Viewer> observer_viewer
    );

    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;
    void apply(vsg::FrameEvent& frame) override;

private:
    void select_object(
        vsg::ref_ptr<vsg::MatrixTransform> object,
        vsg::ref_ptr<SwitchGroup> switch_group,
        vsg::ref_ptr<vsg::PagedLOD> paged_lod
    );

    SelectedObjectsIterator deselect_object(
        vsg::ref_ptr<vsg::MatrixTransform> object
    );

private:
    const settings_t& settings;
    vsg::ref_ptr<KeyboardHandler> keyboard_handler;
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    vsg::ref_ptr<SceneGraph> scene_graph;
    vsg::observer_ptr<vsg::Viewer> observer_viewer;

    SelectedObjectsMap selected_objects;
    vsg::ref_ptr<Gizmo> gizmo;
    vsg::ref_ptr<SingleSwitch> gizmo_switch;
};

#endif // OBJECT_SELECTOR_H
