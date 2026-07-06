#include "editor/states/NavigationState.h"

#include "editor/Mouse.h"
#include "editor/StateManager.h"
#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

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
    if (!(mouse->get_button_mask() & vsg::BUTTON_MASK_3))
    {
        state_manager.defer_switch(EDITOR_STATE_BASIC);
    }
}

void NavigationState::handle_mouse_move()
{
}

void NavigationState::fill_status_bar() const
{
    ImGui::Text("Navigation state");
}
