#include "editor/StateManager.h"

#include "editor/states/BasicEditorState.h"
#include "editor/states/BoxSelectionState.h"
#include "editor/states/EditorState.h"
#include "editor/states/NavigationState.h"
#include "editor/states/RouteNotLoadedState.h"

#include <Journal.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>

#include <memory>
#include <string>

StateManager::StateManager(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    const vsg::ref_ptr<Camera>& camera,
    const std::string& route_dir,
    const vsg::ref_ptr<vsg::Options>& vsg_options
)
{
    editor_states[EDITOR_STATE_ROUTE_NOT_LOADED] =
        std::make_unique<RouteNotLoadedState>(mouse, keyboard, *this);

    editor_states[EDITOR_STATE_BASIC] = std::make_unique<BasicEditorState>(
        mouse, keyboard, *this, route_dir);

    editor_states[EDITOR_STATE_NAVIGATION] = std::make_unique<NavigationState>(
        mouse, keyboard, *this, camera);

    editor_states[EDITOR_STATE_BOX_SELECTION] =
        std::make_unique<BoxSelectionState>(mouse, keyboard, *this,
            vsg_options);

    current_state_index = deferred_state_index = EDITOR_STATE_ROUTE_NOT_LOADED;
}

StateManager::~StateManager() = default;

void StateManager::defer_switch(EnumEditorState state)
{
    deferred_state_index = state;
}

void StateManager::update(double delta_time)
{
    const auto& current_state = editor_states[current_state_index];

    if (current_state_index != deferred_state_index)
    {
        const auto& deferred_state = editor_states[deferred_state_index];

        Journal::instance()->info(QString("'%1' -> '%2'")
            .arg(current_state->get_name())
            .arg(deferred_state->get_name()));

        current_state->on_deactivate();
        current_state_index = deferred_state_index;
        deferred_state->on_activate();
    }

    current_state->update(delta_time);
}

const std::unique_ptr<EditorState>& StateManager::get_editor_state() const
{
    return editor_states[current_state_index];
}
