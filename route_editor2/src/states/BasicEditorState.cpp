#include "editor/states/BasicEditorState.h"

#include "editor/Mouse.h"
#include "editor/StateManager.h"
#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/ui/PointerEvent.h>
#include <vsgImGui/imgui.h>

#include <string>

BasicEditorState::BasicEditorState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager,
    const std::string& route_dir
)
    : EditorState(mouse, keyboard, state_manager)
    , route_dir(route_dir)
{
}

BasicEditorState::~BasicEditorState() = default;

void BasicEditorState::handle_key_press() const
{
}

void BasicEditorState::handle_key_release() const
{
}

void BasicEditorState::handle_button_press() const
{
    switch (mouse->get_button_mask())
    {
        case vsg::BUTTON_MASK_1:
        {
            state_manager.defer_switch(EDITOR_STATE_BOX_SELECTION);
            return;
        }
        case vsg::BUTTON_MASK_3:
        {
            state_manager.defer_switch(EDITOR_STATE_NAVIGATION);
            return;
        }
        default:
        {
            return;
        }
    }
}

void BasicEditorState::handle_button_release() const
{
}

void BasicEditorState::handle_mouse_move()
{
}

void BasicEditorState::update(double delta_time) const
{
    static_cast<void>(delta_time);
}

void BasicEditorState::fill_status_bar() const
{
    ImGui::Text("Basic editor state");
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::Text("Current route: %s\n", route_dir.c_str());
}
