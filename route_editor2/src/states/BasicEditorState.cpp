#include "editor/states/BasicEditorState.h"

#include <vsgImGui/imgui.h>

#include <string>

BasicEditorState::BasicEditorState(const std::string& route_dir)
    : route_dir(route_dir)
{
}

BasicEditorState::~BasicEditorState() = default;

void BasicEditorState::fill_status_bar() const
{
    ImGui::Text("Current route: %s\n", route_dir.c_str());
}
