#include "editor/states/BasicEditorState.h"

#include <vsg/core/ref_ptr.h>
#include <vsg/ui/KeyEvent.h>
#include <vsgImGui/imgui.h>

#include <string>

BasicEditorState::BasicEditorState(const vsg::ref_ptr<Keyboard>& keyboard,
    const std::string& route_dir)
    : keyboard(keyboard)
    , route_dir(route_dir)
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
