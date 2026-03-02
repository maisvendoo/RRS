#ifndef OBJECT_SELECTOR_H
#define OBJECT_SELECTOR_H

#include "EditorContext.h"
#include "RouteObject.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

class Gizmo;
class RouteObject;
class SingleSwitch;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class FrameEvent;
class MoveEvent;

}

class ObjectSelector : public vsg::Inherit<vsg::Visitor, ObjectSelector>
{
public:
    ObjectSelector(EditorContext& context);

    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;
    void apply(vsg::FrameEvent& frame) override;

private:
    void select_object(RouteObject* object);

    void confirm_keyboard_move();
    void cancel_keyboard_move();

    void confirm_keyboard_rotate();
    void cancel_keyboard_rotate();

    void confirm_keyboard_scale();
    void cancel_keyboard_scale();

private:
    enum class State
    {
        INITIAL,
        KEYBOARD_GRAB,
        KEYBOARD_ROTATE,
        KEYBOARD_SCALE
    };

    State state = State::INITIAL;

    EditorContext& context;

    vsg::vec3 begin_intersect_pos;
    vsg::vec3 prev_intersect_pos;
    vsg::vec3 total_translation;
    vsg::ref_ptr<SingleSwitch> front_plane_switch;
    vsg::vec3 front_plane_up;
    vsg::vec3 front;
};

#endif // OBJECT_SELECTOR_H
