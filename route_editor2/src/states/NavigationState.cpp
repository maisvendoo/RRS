#include "editor/states/NavigationState.h"

#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>

NavigationState::NavigationState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : EditorState(mouse, keyboard, state_manager)
{
}

NavigationState::~NavigationState() = default;

void NavigationState::handle_key_press() const
{
}

void NavigationState::handle_key_release() const
{
}

void NavigationState::handle_button_release() const
{
}

void NavigationState::handle_mouse_move()
{
}

void NavigationState::fill_status_bar() const
{
}
