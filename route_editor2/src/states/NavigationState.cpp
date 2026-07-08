#include "editor/states/NavigationState.h"

#include "editor/Camera.h"
#include "editor/Mouse.h"
#include "editor/StateManager.h"
#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>
#include <vsgImGui/imgui.h>

NavigationState::NavigationState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager,
    const vsg::ref_ptr<Camera>& camera
)
    : EditorState(mouse, keyboard, state_manager)
    , camera(camera)
{
    name = "NavigationState";
}

NavigationState::~NavigationState() = default;

void NavigationState::on_activate()
{
    camera->update_move_direction();
}

void NavigationState::handle_key_press() const
{
    camera->update_move_direction();
}

void NavigationState::handle_key_release() const
{
    camera->update_move_direction();
}

void NavigationState::handle_button_release() const
{
    if (!(mouse->get_button_mask() & vsg::BUTTON_MASK_3))
    {
        state_manager.defer_switch_to_basic_editor_state();
    }
}

void NavigationState::handle_mouse_move()
{
    camera->handle_mouse_move();
}

void NavigationState::update(double delta_time) const
{
    camera->update(delta_time);
}

void NavigationState::fill_status_bar() const
{
    ImGui::Text("Navigation state");
}
