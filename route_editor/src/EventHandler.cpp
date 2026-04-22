#include "EventHandler.h"

#include "Keyboard.h"
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

EventHandler::EventHandler(Keyboard* keyboard)
    : keyboard_(keyboard)
    , select_route_state_(new SelectRouteState)
    , initial_state_(new InitialState)
    , camera_navigation_state_(new CameraNavigationState)
    , keyboard_translate_state_(new KeyboardTranslateState)
    , keyboard_rotate_state_(new KeyboardRotateState)
    , keyboard_scale_state_(new KeyboardScaleState)
    , gizmo_translate_state_(new GizmoTranslateState)
    , gizmo_rotate_state_(new GizmoRotateState)
    , gizmo_scale_state_(new GizmoScaleState)
{
    state_ = select_route_state_;
}

EventHandler::~EventHandler()
{
    delete select_route_state_;
    delete initial_state_;
    delete camera_navigation_state_;
    delete keyboard_translate_state_;
    delete keyboard_rotate_state_;
    delete keyboard_scale_state_;
    delete gizmo_translate_state_;
    delete gizmo_rotate_state_;
    delete gizmo_scale_state_;
}

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    keyboard_->handle_key_press(keyPress);
    state_->handle_key_press(keyPress);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    keyboard_->handle_key_release(keyRelease);
}
