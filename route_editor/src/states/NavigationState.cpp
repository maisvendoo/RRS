#include "states/NavigationState.h"

#include "Camera.h"
#include "Mouse.h"
#include "StateManager.h"

#include <vsgImGui/imgui.h>

NavigationState::NavigationState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager,
    const vsg::ref_ptr<Camera>& camera
)
    : State(mouse, keyboard, state_manager)
    , camera(camera)
{
}

NavigationState::~NavigationState() = default;

void NavigationState::on_activate()
{
    camera->update_move_direction();
}

void NavigationState::handle_key_press()
{
    camera->update_move_direction();
}

void NavigationState::handle_key_release()
{
    camera->update_move_direction();
}

void NavigationState::handle_button_release()
{
    if (!mouse->is_rmb_pressed())
    {
        state_manager.defer_switch_to(STATE_BASIC);
    }
}

void NavigationState::handle_mouse_move()
{
    camera->handle_mouse_move();
}

void NavigationState::update(double delta_time)
{
    camera->update(delta_time);
}

void NavigationState::fill_status_bar() const
{
    ImGui::Text("NavigationState");
}

const char* NavigationState::get_name() const
{
    return "NavigationState";
}
