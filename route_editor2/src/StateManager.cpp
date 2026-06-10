#include "editor/StateManager.h"

#include "editor/states/BasicEditorState.h"
#include "editor/states/EditorState.h"
#include "editor/states/RouteNotSelectedState.h"

#include <memory>

StateManager::StateManager()
{
    route_not_selected_state = std::make_unique<RouteNotSelectedState>();
    basic_editor_state = std::make_unique<BasicEditorState>();
    editor_state = &route_not_selected_state;
}

StateManager::~StateManager() = default;

const std::unique_ptr<EditorState>& StateManager::get_editor_state() const
{
    return *editor_state;
}

void StateManager::set_state_route_not_selected()
{
    editor_state = &route_not_selected_state;
}

void StateManager::set_state_basic_editor_state()
{
    editor_state = &basic_editor_state;
}
