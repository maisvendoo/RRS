#include "states/RouteNotLoadedState.h"

#include "ImGuiFileDialog.h"

#include <filesystem.h>

#include <vsgImGui/imgui.h>

RouteNotLoadedState::RouteNotLoadedState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
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

const char* RouteNotLoadedState::get_name() const
{
    return "RouteNotLoadedState";
}
