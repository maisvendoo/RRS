#include "StateManager.h"

#include "states/NavigationState.h"
#include "states/GizmoRotateState.h"
#include "states/GizmoScaleState.h"
#include "states/GizmoTranslateState.h"
#include "states/BasicEditorState.h"
#include "states/KeyboardRotateState.h"
#include "states/KeyboardScaleState.h"
#include "states/KeyboardTranslateState.h"
#include "states/RouteNotLoadedState.h"
#include "states/State.h"

#include <Journal.h>

#include <vsg/core/ref_ptr.h>

#include <QString>

#include <memory>

StateManager::StateManager(
    const vsg::ref_ptr<Keyboard>& keyboard,
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Camera>& camera,
    CommandList& command_list
)
{
    states[STATE_ROUTE_NOT_LOADED] = std::make_unique<RouteNotLoadedState>(mouse, keyboard, *this);
    states[STATE_BASIC] = std::make_unique<BasicEditorState>(mouse, keyboard, *this, camera, command_list);
    states[STATE_NAVIGATION] = std::make_unique<NavigationState>(mouse, keyboard, *this, camera);
    states[STATE_KEYBOARD_TRANSLATE] = std::make_unique<KeyboardTranslateState>(mouse, keyboard, *this);
    states[STATE_KEYBOARD_ROTATE] = std::make_unique<KeyboardRotateState>(mouse, keyboard, *this);
    states[STATE_KEYBOARD_SCALE] = std::make_unique<KeyboardScaleState>(mouse, keyboard, *this);
    states[STATE_GIZMO_TRANSLATE] = std::make_unique<GizmoTranslateState>(mouse, keyboard, *this);
    states[STATE_GIZMO_ROTATE] = std::make_unique<GizmoRotateState>(mouse, keyboard, *this);
    states[STATE_GIZMO_SCALE] = std::make_unique<GizmoScaleState>(mouse, keyboard, *this);

    current_state = &states[STATE_ROUTE_NOT_LOADED];
    deferred_state = &states[STATE_ROUTE_NOT_LOADED];
}

StateManager::~StateManager() = default;

void StateManager::defer_switch_to(StateEnum state)
{
    deferred_state = &states[state];
}

void StateManager::update(double delta_time)
{
    if (current_state != deferred_state)
    {
        Journal::instance()->info(QString("'%1' -> '%2'")
            .arg((*current_state)->get_name())
            .arg((*deferred_state)->get_name()));

        (*current_state)->on_deactivate();
        current_state = deferred_state;
        (*deferred_state)->on_activate();
    }

    (*current_state)->update(delta_time);
}

const std::unique_ptr<State>& StateManager::get_editor_state() const
{
    return *current_state;
}

