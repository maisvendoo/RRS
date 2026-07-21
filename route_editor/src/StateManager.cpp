#include "StateManager.h"

#include "states/NavigationState.h"
#include "states/GizmoRotateState.h"
#include "states/GizmoScaleState.h"
#include "states/GizmoTranslateState.h"
#include "states/InitialState.h"
#include "states/KeyboardRotateState.h"
#include "states/KeyboardScaleState.h"
#include "states/KeyboardTranslateState.h"
#include "states/SelectRouteState.h"

StateManager::StateManager()
{
    select_route_state = new SelectRouteState;
    initial_state = new InitialState;
    navigation_state = new NavigationState;
    keyboard_translate_state = new KeyboardTranslateState;
    keyboard_rotate_state = new KeyboardRotateState;
    keyboard_scale_state = new KeyboardScaleState;
    gizmo_translate_state = new GizmoTranslateState;
    gizmo_rotate_state = new GizmoRotateState;
    gizmo_scale_state = new GizmoScaleState;

    current_state = select_route_state;
    deferred_state = select_route_state;
}

StateManager::~StateManager()
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

void StateManager::defer_switch_to_route_not_loaded_state()
{
    deferred_state = select_route_state;
}

void StateManager::defer_switch_to_basic_editor_state()
{
    deferred_state = initial_state;
}

void StateManager::defer_switch_to_navigation_state()
{
    deferred_state = navigation_state;
}

void StateManager::defer_switch_to_box_selection_state()
{
    // TODO
}

void StateManager::defer_switch_to_keyboard_translate_state()
{
    deferred_state = keyboard_translate_state;
}

void StateManager::defer_switch_to_keyboard_rotate_state()
{
    deferred_state = keyboard_rotate_state;
}

void StateManager::defer_switch_to_keyboard_scale_state()
{
    deferred_state = keyboard_scale_state;
}

void StateManager::defer_switch_to_gizmo_translate_state()
{
    deferred_state = gizmo_translate_state;
}

void StateManager::defer_switch_to_gizmo_rotate_state()
{
    deferred_state = gizmo_rotate_state;
}

void StateManager::defer_switch_to_gizmo_scale_state()
{
    deferred_state = gizmo_scale_state;
}

void StateManager::update(double delta_time)
{
    if (current_state != deferred_state)
    {
        // TODO
    }
    // TODO
}

State* StateManager::get_editor_state() const
{
    return current_state;
}

