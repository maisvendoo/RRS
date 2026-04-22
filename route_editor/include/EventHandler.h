#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

class CameraNavigationState;
class GizmoRotateState;
class GizmoScaleState;
class GizmoTranslateState;
class InitialState;
class Keyboard;
class KeyboardRotateState;
class KeyboardScaleState;
class KeyboardTranslateState;
class SelectRouteState;
class State;

namespace vsg
{

class KeyPressEvent;
class KeyReleaseEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(Keyboard* keyboard);
    virtual ~EventHandler() override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

private:
    Keyboard* const keyboard_;

    State* state_;
    SelectRouteState* const select_route_state_;
    InitialState* const initial_state_;
    CameraNavigationState* const camera_navigation_state_;
    KeyboardTranslateState* const keyboard_translate_state_;
    KeyboardRotateState* const keyboard_rotate_state_;
    KeyboardScaleState* const keyboard_scale_state_;
    GizmoTranslateState* const gizmo_translate_state_;
    GizmoRotateState* const gizmo_rotate_state_;
    GizmoScaleState* const gizmo_scale_state_;
};

#endif // EVENT_HANDLER_H
