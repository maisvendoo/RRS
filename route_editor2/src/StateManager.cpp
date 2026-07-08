#include "editor/StateManager.h"

#include "editor/states/BasicEditorState.h"
#include "editor/states/BoxSelectionState.h"
#include "editor/states/EditorState.h"
#include "editor/states/NavigationState.h"
#include "editor/states/RouteNotLoadedState.h"

#include <Journal.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>

#include <new>
#include <string>

StateManager::StateManager(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    const vsg::ref_ptr<Camera>& camera,
    const std::string& route_dir,
    const vsg::ref_ptr<vsg::Options>& vsg_options,
    const vsg::ref_ptr<vsg::Group>& gui_group
)
{
    route_not_loaded_state = new RouteNotLoadedState(mouse, keyboard, *this);
    basic_editor_state = new BasicEditorState(mouse, keyboard, *this, route_dir);
    navigation_state = new NavigationState(mouse, keyboard, *this, camera);
    box_selection_state = new BoxSelectionState(mouse, keyboard, *this,
        vsg_options, gui_group);

    current_state = deferred_state = route_not_loaded_state;
}

StateManager::~StateManager()
{
    delete box_selection_state;
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
    deferred_state = box_selection_state;
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

EditorState* StateManager::get_editor_state() const
{
    return current_state;
}
