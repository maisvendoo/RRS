#include "states/RouteNotLoadedState.h"

#include "ImGuiFileDialog.h"

#include <filesystem.h>

#include <vsgImGui/imgui.h>

RouteNotLoadedState::RouteNotLoadedState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
    name = "RouteNotLoadedState";
}

RouteNotLoadedState::~RouteNotLoadedState() = default;

void RouteNotLoadedState::fill_status_bar() const
{
    State::fill_status_bar();
    if (ImGui::Button("Load route"))
    {
        IGFD::FileDialogConfig config;
        config.path = FileSystem::getInstance().getRouteRootDir();
        ImGuiFileDialog::Instance()->OpenDialog("LoadRouteKey",
            "Load route", nullptr, config);
    }
}
