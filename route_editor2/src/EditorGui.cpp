#include "editor/EditorGui.h"

#include "editor/settings/GuiSettings.h"

#include <filesystem.h>

#include <ImGuiFileDialog.h>
#include <vsgImGui/imgui.h>
#include <vsgImGui/imgui_internal.h>

#include <cfloat>
#include <filesystem>
#include <string>

EditorGui::EditorGui(const gui_settings_t& gui_settings)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    const FileSystem& fs = FileSystem::getInstance();

    const char* const font_name = "JetBrainsMono-Regular.ttf";
    const std::string font_path = fs.combinePath(fs.getFontsDir(), font_name);

    io.Fonts->AddFontFromFileTTF(font_path.c_str(), gui_settings.font_size);
}

void EditorGui::record(vsg::CommandBuffer&) const
{
    ImGui::ShowDemoWindow();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New route"))
            {
                // TODO
            }

            if (ImGui::MenuItem("Open route"))
            {
                IGFD::FileDialogConfig config;
                config.path = FileSystem::getInstance().getRouteRootDir();
                ImGuiFileDialog::Instance()->OpenDialog("OpenRouteKey",
                    "Open route", nullptr, config);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGuiViewport* const viewport = ImGui::GetMainViewport();

    if (ImGui::BeginViewportSideBar("StatusBar", viewport, ImGuiDir_Down,
        ImGui::GetFrameHeight(), ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::Text("Status bar");
        ImGui::End();
    }

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGuiFileDialog::Instance()->Display("OpenRouteKey",
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            const std::string route_dir = ImGuiFileDialog::Instance()->GetCurrentPath();
            if (!std::filesystem::exists(route_dir + "/models") ||
                !std::filesystem::exists(route_dir + "/textures") ||
                !std::filesystem::exists(route_dir + "/topology") ||
                !std::filesystem::exists(route_dir + "/objects.ref"))
            {
                ImGui::OpenPopup("InvalidRoute");
            }
            else
            {
                ImGuiFileDialog::Instance()->Close();
            }
        }
        else
        {
            ImGuiFileDialog::Instance()->Close();
        }
    }

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

