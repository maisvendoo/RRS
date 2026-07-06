#include "editor/states/RouteNotLoadedState.h"

#include "editor/states/EditorState.h"

#include <filesystem.h>

#include <ImGuiFileDialog.h>
#include <vsgImGui/imgui.h>

RouteNotLoadedState::RouteNotLoadedState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : EditorState(mouse, keyboard, state_manager)
{
}

RouteNotLoadedState::~RouteNotLoadedState() = default;

void RouteNotLoadedState::fill_status_bar() const
{
    ImGui::Text("Route is not loaded");
    ImGui::SameLine();
    if (ImGui::Button("Load route"))
    {
        IGFD::FileDialogConfig config;
        config.path = FileSystem::getInstance().getRouteRootDir();
        ImGuiFileDialog::Instance()->OpenDialog("LoadRouteKey",
            "Load route", nullptr, config);
    }
}
