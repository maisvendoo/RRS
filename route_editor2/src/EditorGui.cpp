#include "editor/EditorGui.h"

#include "editor/settings/GuiSettings.h"

#include <filesystem.h>

#include <vsgImGui/imgui.h>

#include <string>

EditorGui::EditorGui(const gui_settings_t& gui_settings)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

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
                // TODO
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

