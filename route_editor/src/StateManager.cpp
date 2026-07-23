#include "StateManager.h"

#include "Journal.h"
#include "states/NavigationState.h"
#include "states/GizmoRotateState.h"
#include "states/GizmoScaleState.h"
#include "states/GizmoTranslateState.h"
#include "states/BasicEditorState.h"
#include "states/KeyboardRotateState.h"
#include "states/KeyboardScaleState.h"
#include "states/KeyboardTranslateState.h"
#include "states/RouteNotLoadedState.h"

#include <vsg/core/ref_ptr.h>

StateManager::StateManager(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    const vsg::ref_ptr<Camera>& camera
)
{
    route_not_loaded_state = new RouteNotLoadedState(mouse, keyboard, *this);
    basic_editor_state = new BasicEditorState(mouse, keyboard, *this, camera);
    navigation_state = new NavigationState(mouse, keyboard, *this, camera);
    keyboard_translate_state = new KeyboardTranslateState(mouse, keyboard, *this);
    keyboard_rotate_state = new KeyboardRotateState(mouse, keyboard, *this);
    keyboard_scale_state = new KeyboardScaleState(mouse, keyboard, *this);
    gizmo_translate_state = new GizmoTranslateState(mouse, keyboard, *this);
    gizmo_rotate_state = new GizmoRotateState(mouse, keyboard, *this);
    gizmo_scale_state = new GizmoScaleState(mouse, keyboard, *this);

    current_state = route_not_loaded_state;
    deferred_state = route_not_loaded_state;
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
    delete basic_editor_state;
    delete route_not_loaded_state;
}

void StateManager::defer_switch_to_route_not_loaded_state()
{
    deferred_state = route_not_loaded_state;
}

void StateManager::defer_switch_to_basic_editor_state()
{
    deferred_state = basic_editor_state;
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
        Journal::instance()->info(QString("'%1' -> '%2'")
            .arg(current_state->get_name())
            .arg(deferred_state->get_name()));

        current_state->on_deactivate();
        current_state = deferred_state;
        deferred_state->on_activate();
    }

    current_state->update(delta_time);
}

State* StateManager::get_editor_state() const
{
    return current_state;
}

