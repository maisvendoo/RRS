#include "states/BasicEditorState.h"

#include "Action.h"
#include "Camera.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "StateManager.h"
#include "commands/CommandList.h"

#include <vsg/ui/PointerEvent.h>
#include <vsgImGui/imgui.h>

BasicEditorState::BasicEditorState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager,
    const vsg::ref_ptr<Camera>& camera,
    CommandList& command_list
)
    : State(mouse, keyboard, state_manager)
    , camera(camera)
    , command_list(command_list)
{
}

BasicEditorState::~BasicEditorState() = default;

void BasicEditorState::handle_key_press()
{
    if (keyboard->pressed_once(ACTION_UNDO_COMMAND))
    {
        command_list.undo();
    }
    else if (keyboard->pressed_once(ACTION_REDO_COMMAND))
    {
        command_list.redo();
    }
    else
    {
        camera->handle_key_press();
    }
}

void BasicEditorState::handle_button_press()
{
    switch (mouse->get_button_mask())
    {
        case vsg::BUTTON_MASK_1:
        {
            return;
        }
        case vsg::BUTTON_MASK_3:
        {
            state_manager.defer_switch_to(STATE_NAVIGATION);
            return;
        }
        default:
        {
            return;
        }
    }
}

void BasicEditorState::handle_mouse_scroll()
{
    camera->handle_mouse_scroll();
}

void BasicEditorState::fill_status_bar() const
{
    ImGui::Text("BasicEditorState");
}

const char* BasicEditorState::get_name() const
{
    return "BasicEditorState";
}
