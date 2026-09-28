#include "editor/StateManager.h"

#include "editor/EditorContext.h"
#include "editor/states/NavigationState.h"
#include "editor/states/GizmoRotateState.h"
#include "editor/states/GizmoScaleState.h"
#include "editor/states/GizmoTranslateState.h"
#include "editor/states/BasicEditorState.h"
#include "editor/states/KeyboardRotateState.h"
#include "editor/states/KeyboardScaleState.h"
#include "editor/states/KeyboardTranslateState.h"
#include "editor/states/RouteNotLoadedState.h"
#include "editor/states/State.h"

#include <Journal.h>

#include <vsg/core/ref_ptr.h>

#include <QString>

#include <memory>

StateManager::StateManager(EditorContext& editor_context)
{
    states[STATE_ROUTE_NOT_LOADED] = std::make_unique<RouteNotLoadedState>(editor_context);
    states[STATE_BASIC] = std::make_unique<BasicEditorState>(editor_context);
    states[STATE_NAVIGATION] = std::make_unique<NavigationState>(editor_context);
    states[STATE_KEYBOARD_TRANSLATE] = std::make_unique<KeyboardTranslateState>(editor_context);
    states[STATE_KEYBOARD_ROTATE] = std::make_unique<KeyboardRotateState>(editor_context);
    states[STATE_KEYBOARD_SCALE] = std::make_unique<KeyboardScaleState>(editor_context);
    states[STATE_GIZMO_TRANSLATE] = std::make_unique<GizmoTranslateState>(editor_context);
    states[STATE_GIZMO_ROTATE] = std::make_unique<GizmoRotateState>(editor_context);
    states[STATE_GIZMO_SCALE] = std::make_unique<GizmoScaleState>(editor_context);

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

const std::unique_ptr<State>& StateManager::get_current_editor_state() const
{
    return *current_state;
}

RouteNotLoadedState* StateManager::get_route_not_loaded_state() const
{
    return dynamic_cast<RouteNotLoadedState*>(states[STATE_ROUTE_NOT_LOADED].get());
}

BasicEditorState* StateManager::get_basic_editor_state() const
{
    return dynamic_cast<BasicEditorState*>(states[STATE_BASIC].get());
}

NavigationState* StateManager::get_navigation_state() const
{
    return dynamic_cast<NavigationState*>(states[STATE_NAVIGATION].get());
}

KeyboardTranslateState* StateManager::get_keyboard_translate_state() const
{
    return dynamic_cast<KeyboardTranslateState*>(states[STATE_KEYBOARD_TRANSLATE].get());
}

KeyboardRotateState* StateManager::get_keyboard_rotate_state() const
{
    return dynamic_cast<KeyboardRotateState*>(states[STATE_KEYBOARD_ROTATE].get());
}

KeyboardScaleState* StateManager::get_keyboard_scale_state() const
{
    return dynamic_cast<KeyboardScaleState*>(states[STATE_KEYBOARD_SCALE].get());
}

GizmoTranslateState* StateManager::get_gizmo_translate_state() const
{
    return dynamic_cast<GizmoTranslateState*>(states[STATE_GIZMO_TRANSLATE].get());
}

GizmoRotateState* StateManager::get_gizmo_rotate_state() const
{
    return dynamic_cast<GizmoRotateState*>(states[STATE_GIZMO_ROTATE].get());
}

GizmoScaleState* StateManager::get_gizmo_scale_state() const
{
    return dynamic_cast<GizmoScaleState*>(states[STATE_GIZMO_SCALE].get());
}

