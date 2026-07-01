#include "editor/states/BasicEditorState.h"

#include "editor/Camera.h"
#include "editor/Keyboard.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/ui/KeyEvent.h>
#include <vsgImGui/imgui.h>

#include <string>

BasicEditorState::BasicEditorState(
    const vsg::ref_ptr<Keyboard>& keyboard,
    const std::string& route_dir,
    const vsg::ref_ptr<Camera>& camera
)
    : keyboard(keyboard)
    , route_dir(route_dir)
    , camera(camera)
{
}

BasicEditorState::~BasicEditorState() = default;

void BasicEditorState::fill_status_bar() const
{
    ImGui::Text("Current route: %s\n", route_dir.c_str());
}

void BasicEditorState::handle_key_press(vsg::KeySymbol key) const
{
}

void BasicEditorState::handle_key_release(vsg::KeySymbol key) const
{
}

void BasicEditorState::handle_mouse_move(int delta_x, int delta_y) const
{
    camera->handle_mouse_move(delta_x, delta_y);
}

void BasicEditorState::update(double delta_time) const
{
    camera->update(delta_time);
}
