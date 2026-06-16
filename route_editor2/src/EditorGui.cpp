#include "editor/EditorGui.h"

#include "editor/RouteLoader.h"
#include "editor/StateManager.h"
#include "editor/settings/GuiSettings.h"
#include "editor/states/EditorState.h"

#include <filesystem.h>

#include <ImGuiFileDialog.h>
#include <vsgImGui/imgui.h>
#include <vsgImGui/imgui_internal.h>

#include <cfloat>
#include <filesystem>
#include <string>

EditorGui::EditorGui(
    const gui_settings_t& gui_settings,
    StateManager& state_manager,
    std::string& route_dir,
    RouteLoader& route_loader
)
    : state_manager(state_manager)
    , route_dir(route_dir)
    , route_loader(route_loader)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    add_ttf_font("JetBrainsMono-Regular.ttf", gui_settings.font_size);

    viewport = ImGui::GetMainViewport();
}

void EditorGui::record(vsg::CommandBuffer&) const
{
    ImGui::ShowDemoWindow();

    draw_menu_bar();
    draw_status_bar();
    draw_load_route_file_dialog();
    draw_invalid_route_popup();
}

void EditorGui::add_ttf_font(const char* filename, float size)
{
    ImGuiIO& io = ImGui::GetIO();
    const FileSystem& fs = FileSystem::getInstance();
    const std::string font_path = fs.combinePath(fs.getFontsDir(), filename);
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), size);
}

void EditorGui::draw_menu_bar() const
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New route"))
            {
                // TODO
            }

            if (ImGui::MenuItem("Load route"))
            {
                IGFD::FileDialogConfig config;
                config.path = FileSystem::getInstance().getRouteRootDir();
                ImGuiFileDialog::Instance()->OpenDialog("LoadRouteKey",
                    "Load route", nullptr, config);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EditorGui::draw_status_bar() const
{
    if (ImGui::BeginViewportSideBar("StatusBar", viewport, ImGuiDir_Down,
        ImGui::GetFrameHeight() * 1.4f, ImGuiWindowFlags_NoScrollbar))
    {
        state_manager.get_editor_state()->fill_status_bar();
        ImGui::End();
    }
}

void EditorGui::draw_load_route_file_dialog() const
{
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGuiFileDialog::Instance()->Display("LoadRouteKey",
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
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
                state_manager.defer_switch_to_basic_editor_state();
                route_loader.start_load_route(route_dir);
                ImGuiFileDialog::Instance()->Close();
            }
        }
        else
        {
            ImGuiFileDialog::Instance()->Close();
        }
    }
}

void EditorGui::draw_invalid_route_popup() const
{
    if (ImGui::BeginPopupModal("InvalidRoute", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
    {
        ImGui::Text(
            "Invalid route!\n"
            "Route must contain:\n"
            "models/\n"
            "textures/\n"
            "topology/\n"
            "objects.ref"
        );

        if (ImGui::Button("OK", ImVec2(-FLT_MIN, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
