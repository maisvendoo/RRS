#include "editor/states/RouteNotLoadedState.h"

#include <filesystem.h>

#include <ImGuiFileDialog.h>
#include <vsgImGui/imgui.h>

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
