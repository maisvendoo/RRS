#include "editor/StateManager.h"

#include "editor/states/BasicEditorState.h"
#include "editor/states/BoxSelectionState.h"
#include "editor/states/EditorState.h"
#include "editor/states/NavigationState.h"
#include "editor/states/RouteNotLoadedState.h"

#include <Journal.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>

#include <array>
#include <memory>
#include <string>

using EditorStateNames = std::array<const char*, TOTAL_EDITOR_STATES>;

static EditorStateNames get_editor_state_names();

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
        mouse, keyboard, *this, camera, route_dir);

    editor_states[EDITOR_STATE_NAVIGATION] = std::make_unique<NavigationState>(
        mouse, keyboard, *this);

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
    if (current_state_index != deferred_state_index)
    {
        static EditorStateNames editor_state_names = get_editor_state_names();
        Journal::instance()->info(QString("state manager: switch to '%1'")
            .arg(editor_state_names[deferred_state_index]));
        current_state_index = deferred_state_index;
        editor_states[current_state_index]->on_activate();
    }
    editor_states[current_state_index]->update(delta_time);
}

const std::unique_ptr<EditorState>& StateManager::get_editor_state() const
{
    return editor_states[current_state_index];
}

EditorStateNames get_editor_state_names()
{
    EditorStateNames editor_state_names;

    editor_state_names[EDITOR_STATE_ROUTE_NOT_LOADED] = "RouteNotLoadedState";
    editor_state_names[EDITOR_STATE_BASIC] = "BasicEditorState";
    editor_state_names[EDITOR_STATE_NAVIGATION] = "NavigationState";
    editor_state_names[EDITOR_STATE_BOX_SELECTION] = "BoxSelectionState";

    return editor_state_names;
}
