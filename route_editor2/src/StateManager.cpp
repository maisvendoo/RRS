#include "editor/StateManager.h"

#include "editor/Mouse.h"
#include "editor/states/BasicEditorState.h"
#include "editor/states/BoxSelectionState.h"
#include "editor/states/EditorState.h"
#include "editor/states/RouteNotLoadedState.h"

#include <Journal.h>

#include <vsg/core/ref_ptr.h>

#include <memory>
#include <string>

StateManager::StateManager(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    const std::string& route_dir,
    const vsg::ref_ptr<Camera>& camera
)
    : mouse(mouse)
{
    route_not_loaded_state = std::make_unique<RouteNotLoadedState>();
    basic_editor_state = std::make_unique<BasicEditorState>(
        mouse, keyboard, route_dir, camera, *this);
    box_selection_state = std::make_unique<BoxSelectionState>(
        mouse, *this
    );
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

    Journal::instance()->info("Deferred switch to state 'RouteNotLoaded'");
}

void StateManager::defer_switch_to_basic_editor_state()
{
    deferred_editor_state = &basic_editor_state;

    Journal::instance()->info("Deferred switch to state 'BasicEditorState'");
}

void StateManager::defer_switch_to_box_selection_state()
{
    deferred_editor_state = &box_selection_state;

    auto* const state = dynamic_cast<BoxSelectionState*>(box_selection_state.get());
    state->begin_x = state->end_x = mouse->get_pos_x();
    state->begin_y = state->end_y = mouse->get_pos_y();

    Journal::instance()->info("Deferred switch to state 'BoxSelectionState'");
}

void StateManager::update(double delta_time)
{
    editor_state = deferred_editor_state;
    (*editor_state)->update(delta_time);
}
