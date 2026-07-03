#include "editor/states/BasicEditorState.h"

#include "editor/Camera.h"
#include "editor/Keyboard.h"
#include "editor/Mouse.h"
#include "editor/StateManager.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/ui/PointerEvent.h>
#include <vsgImGui/imgui.h>

#include <string>

BasicEditorState::BasicEditorState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    const std::string& route_dir,
    const vsg::ref_ptr<Camera>& camera,
    StateManager& state_manager
)
    : mouse(mouse)
    , keyboard(keyboard)
    , route_dir(route_dir)
    , camera(camera)
    , state_manager(state_manager)
{
}

BasicEditorState::~BasicEditorState() = default;

void BasicEditorState::fill_status_bar() const
{
    ImGui::Text("Basic editor state");
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::Text("Current route: %s\n", route_dir.c_str());
}

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
        state_manager.defer_switch_to_box_selection_state();
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
