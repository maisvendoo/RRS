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

#include <memory>

EventHandler::EventHandler()
    : select_route_state_(std::make_unique<SelectRouteState>())
    , initial_state_(std::make_unique<InitialState>())
    , camera_navigation_state_(std::make_unique<CameraNavigationState>())
    , keyboard_translate_state_(std::make_unique<KeyboardTranslateState>())
    , keyboard_rotate_state_(std::make_unique<KeyboardRotateState>())
    , keyboard_scale_state_(std::make_unique<KeyboardScaleState>())
    , gizmo_translate_state_(std::make_unique<GizmoTranslateState>())
    , gizmo_rotate_state_(std::make_unique<GizmoRotateState>())
    , gizmo_scale_state_(std::make_unique<GizmoScaleState>())
{
    state_ = &select_route_state_;
}

EventHandler::~EventHandler() = default;

void EventHandler::apply(vsg::KeyPressEvent& keyPress)
{
    (*state_)->handle_key_press(keyPress);
}

void EventHandler::apply(vsg::KeyReleaseEvent& keyRelease)
{
    (*state_)->handle_key_release(keyRelease);
}
