#include "EventHandler.h"

#include "states/CameraNavigationState.h"
#include "states/GizmoRotateState.h"
#include "states/GizmoScaleState.h"
#include "states/GizmoTranslateState.h"
#include "states/InitialState.h"
#include "states/KeyboardRotateState.h"
#include "states/KeyboardScaleState.h"
#include "states/KeyboardTranslateState.h"
#include "states/SelectRouteState.h"
#include "states/State.h"

EventHandler::EventHandler()
{
    select_route_state = new SelectRouteState;
    initial_state = new InitialState;
    navigation_state = new CameraNavigationState;
    keyboard_translate_state = new KeyboardTranslateState;
    keyboard_rotate_state = new KeyboardRotateState;
    keyboard_scale_state = new KeyboardScaleState;
    gizmo_translate_state = new GizmoTranslateState;
    gizmo_rotate_state = new GizmoRotateState;
    gizmo_scale_state = new GizmoScaleState;

    current_state = select_route_state;
    deferred_state = select_route_state;
}

EventHandler::~EventHandler()
{
    delete gizmo_scale_state;
    delete gizmo_rotate_state;
    delete gizmo_translate_state;
    delete keyboard_scale_state;
    delete keyboard_rotate_state;
    delete keyboard_translate_state;
    delete navigation_state;
    delete initial_state;
    delete select_route_state;
}

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    current_state->handle_key_press(keyPress);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    current_state->handle_key_release(keyRelease);
}

void EventHandler::apply(vsg::FrameEvent& frameEvent)
{
    (void)frameEvent;
}
