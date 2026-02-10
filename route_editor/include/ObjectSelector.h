#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include "SelectedObjects.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

class CameraHandler;
class CommandList;
class Gizmo;
class IntersectionHandler;
class KeyboardHandler;
class MouseHandler;
class RouteObject;
class SceneGraph;
class SingleSwitch;
struct settings_t;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class FrameEvent;
class MoveEvent;
class Viewer;

}

class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    ObjectSelector(
        const settings_t& settings,
        CommandList& commands,
        vsg::ref_ptr<MouseHandler> mouse_handler,
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

    const SelectedObjects& get_selected_objects() const;

private:
    void select_object(vsg::ref_ptr<RouteObject> object);

    SelectedObjectsIterator deselect_object(vsg::ref_ptr<RouteObject> object);
    void deselect_all_objects();

    void confirm_keyboard_move();
    void cancel_keyboard_move();

private:
    enum class State
    {
        INITIAL,
        KEYBOARD_GRAB,
        KEYBOARD_ROTATE
    };

    State state = State::INITIAL;

    const settings_t& settings;
    CommandList& commands;
    vsg::ref_ptr<MouseHandler> mouse_handler;
    vsg::ref_ptr<KeyboardHandler> keyboard_handler;
    vsg::ref_ptr<CameraHandler> camera_handler;
    vsg::ref_ptr<IntersectionHandler> intersection_handler;
    vsg::ref_ptr<SceneGraph> scene_graph;
    vsg::observer_ptr<vsg::Viewer> observer_viewer;

    SelectedObjects selected_objects;
    vsg::ref_ptr<Gizmo> gizmo;
    vsg::ref_ptr<SingleSwitch> gizmo_switch;

    vsg::vec3 begin_intersection_pos;
    vsg::ref_ptr<SingleSwitch> front_plane_switch;
};

#endif // OBJECT_SELECTOR_H
