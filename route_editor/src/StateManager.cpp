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

#include <vsg/core/ref_ptr.h>

StateManager::StateManager(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
{
    select_route_state = new SelectRouteState(mouse, keyboard);
    initial_state = new InitialState(mouse, keyboard);
    navigation_state = new NavigationState(mouse, keyboard);
    keyboard_translate_state = new KeyboardTranslateState(mouse, keyboard);
    keyboard_rotate_state = new KeyboardRotateState(mouse, keyboard);
    keyboard_scale_state = new KeyboardScaleState(mouse, keyboard);
    gizmo_translate_state = new GizmoTranslateState(mouse, keyboard);
    gizmo_rotate_state = new GizmoRotateState(mouse, keyboard);
    gizmo_scale_state = new GizmoScaleState(mouse, keyboard);

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

