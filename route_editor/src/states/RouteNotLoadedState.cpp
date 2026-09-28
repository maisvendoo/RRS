#include "editor/states/RouteNotLoadedState.h"

#include "editor/EditorContext.h"
#include "editor/Route.h"
#include "editor/StateManager.h"

#include "ImGuiFileDialog.h"

#include <filesystem.h>

#include <vsgImGui/imgui.h>

#include <filesystem>

RouteNotLoadedState::RouteNotLoadedState(EditorContext& editor_context)
    : State(editor_context)
{
    name = "RouteNotLoadedState";
}

RouteNotLoadedState::~RouteNotLoadedState() = default;

void RouteNotLoadedState::draw_gui() const
{
    if (ImGuiFileDialog::Instance()->Display("LoadRouteKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            auto& route_dir = editor_context.route->route_dir;
            route_dir = ImGuiFileDialog::Instance()->GetCurrentPath();
            if (!std::filesystem::exists(route_dir + "/models") ||
                !std::filesystem::exists(route_dir + "/textures") ||
                !std::filesystem::exists(route_dir + "/topology") ||
                !std::filesystem::exists(route_dir + "/objects.ref"))
            {
                ImGui::OpenPopup("InvalidRoute");
            }
            else
            {
                editor_context.state_manager->defer_switch_to(STATE_BASIC);
                editor_context.editor_state = EditorState::LOAD_ROUTE;
                ImGuiFileDialog::Instance()->Close();
            }
        }
        else
        {
            ImGuiFileDialog::Instance()->Close();
        }
    }
}

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
