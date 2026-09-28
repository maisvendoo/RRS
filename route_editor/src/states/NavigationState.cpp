#include "editor/states/NavigationState.h"

#include "editor/Camera.h"
#include "editor/Mouse.h"
#include "editor/EditorContext.h"
#include "editor/StateManager.h"

#include <vsgImGui/imgui.h>

NavigationState::NavigationState(EditorContext& editor_context)
    : State(editor_context)
{
    name = "NavigationState";
}

NavigationState::~NavigationState() = default;

void NavigationState::on_activate()
{
    editor_context.camera->update_move_direction();
}

void NavigationState::handle_key_press()
{
    editor_context.camera->update_move_direction();
}

void NavigationState::handle_key_release()
{
    editor_context.camera->update_move_direction();
}

void NavigationState::handle_button_release()
{
    if (!editor_context.mouse->is_rmb_pressed())
    {
        editor_context.state_manager->defer_switch_to(STATE_BASIC);
    }
}

void NavigationState::handle_mouse_move()
{
    editor_context.camera->handle_mouse_move();
}

void NavigationState::update(double delta_time)
{
    editor_context.camera->update(delta_time);
}
