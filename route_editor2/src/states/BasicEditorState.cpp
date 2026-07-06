#include "editor/states/BasicEditorState.h"

#include "editor/Camera.h"
#include "editor/Mouse.h"
#include "editor/StateManager.h"
#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/ui/PointerEvent.h>
#include <vsgImGui/imgui.h>

#include <string>

BasicEditorState::BasicEditorState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<const Keyboard>& keyboard,
    StateManager& state_manager,
    const vsg::ref_ptr<Camera>& camera,
    const std::string& route_dir
)
    : EditorState(mouse, keyboard, state_manager)
    , camera(camera)
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
    if (mouse->get_button_mask() == vsg::BUTTON_MASK_1)
    {
        state_manager.defer_switch(EDITOR_STATE_BOX_SELECTION);
    }
}

void BasicEditorState::handle_button_release() const
{
}

void BasicEditorState::handle_mouse_move()
{
    camera->handle_mouse_move();
}

void BasicEditorState::update(double delta_time) const
{
    camera->update(delta_time);
}

void BasicEditorState::fill_status_bar() const
{
    ImGui::Text("Basic editor state");
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::Text("Current route: %s\n", route_dir.c_str());
}
