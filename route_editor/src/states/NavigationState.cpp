#include "editor/states/NavigationState.h"

#include "editor/Camera.h"
#include "editor/Mouse.h"
#include "editor/EditorContext.h"
#include "editor/StateManager.h"

#include <vsgImGui/imgui.h>

NavigationState::NavigationState(EditorContext& editor_ctx)
    : State(editor_ctx)
{
    name = "NavigationState";
}

NavigationState::~NavigationState() = default;

void NavigationState::on_activate()
{
    editor_ctx.camera->update_move_dir();
}

void NavigationState::handle_key_press()
{
    editor_ctx.camera->update_move_dir();
}

void NavigationState::handle_key_release()
{
    editor_ctx.camera->update_move_dir();
}

void NavigationState::handle_button_release()
{
    if (!editor_ctx.mouse->is_rmb_pressed())
    {
        editor_ctx.state_manager->defer_switch_to(STATE_BASIC);
    }
}

void NavigationState::handle_mouse_move()
{
    editor_ctx.camera->handle_mouse_move();
}

void NavigationState::update(double delta_time)
{
    editor_ctx.camera->update(delta_time);
}
