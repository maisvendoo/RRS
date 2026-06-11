#include "editor/StateManager.h"

#include "editor/states/BasicEditorState.h"
#include "editor/states/EditorState.h"
#include "editor/states/RouteNotLoadedState.h"

#include <memory>

StateManager::StateManager()
{
    route_not_loaded_state = std::make_unique<RouteNotLoadedState>();
    basic_editor_state = std::make_unique<BasicEditorState>();
    editor_state = &route_not_loaded_state;
    deferred_editor_state = &route_not_loaded_state;
}

StateManager::~StateManager() = default;

const std::unique_ptr<EditorState>& StateManager::get_editor_state() const
{
    return *editor_state;
}

void StateManager::defer_switch_to_route_not_loaded_state()
{
    deferred_editor_state = &route_not_loaded_state;
}

void StateManager::defer_switch_to_basic_editor_state()
{
    deferred_editor_state = &basic_editor_state;
}

void StateManager::update()
{
    editor_state = deferred_editor_state;
}
