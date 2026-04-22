#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "states/CameraNavigationState.h"
#include "states/GizmoRotateState.h"
#include "states/GizmoScaleState.h"
#include "states/GizmoTranslateState.h"
#include "states/InitialState.h"
#include "states/KeyboardRotateState.h"
#include "states/KeyboardScaleState.h"
#include "states/KeyboardTranslateState.h"
#include "states/SelectRouteState.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

class Keyboard;
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
    SelectRouteState select_route_state_;
    InitialState initial_state_;
    CameraNavigationState camera_navigation_state_;
    KeyboardTranslateState keyboard_translate_state_;
    KeyboardRotateState keyboard_rotate_state_;
    KeyboardScaleState keyboard_scale_state_;
    GizmoTranslateState gizmo_translate_state_;
    GizmoRotateState gizmo_rotate_state_;
    GizmoScaleState gizmo_scale_state_;
};

#endif // EVENT_HANDLER_H
