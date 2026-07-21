#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

class CameraNavigationState;
class GizmoRotateState;
class GizmoScaleState;
class GizmoTranslateState;
class InitialState;
class KeyboardRotateState;
class KeyboardScaleState;
class KeyboardTranslateState;
class SelectRouteState;
class State;

namespace vsg
{

class FrameEvent;
class KeyPressEvent;
class KeyReleaseEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    EventHandler();
    virtual ~EventHandler() override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;
    virtual void apply(vsg::FrameEvent& frameEvent) override;

private:
    State* current_state;
    State* deferred_state;

    State* select_route_state;
    State* initial_state;
    State* navigation_state;

    State* keyboard_translate_state;
    State* keyboard_rotate_state;
    State* keyboard_scale_state;

    State* gizmo_translate_state;
    State* gizmo_rotate_state;
    State* gizmo_scale_state;
};

#endif // EVENT_HANDLER_H
