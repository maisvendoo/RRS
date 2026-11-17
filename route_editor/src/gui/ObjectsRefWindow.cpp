#include "editor/gui/ObjectsRefWindow.h"

#include "editor/Camera.h"
#include "editor/EditorContext.h"
#include "editor/RouteObject.h"

#include <vsgImGui/imgui.h>

#include <algorithm>
#include <cctype>
#include <string>

#define SEARCH_BUF_SIZE 256

ObjectsRefWindow::ObjectsRefWindow(EditorContext& editor_ctx,
                                   ImGuiWindowFlags window_flags)
    : camera(editor_ctx.camera)
    , command_manager(editor_ctx.command_manager.get())
    , gui_settings(editor_ctx.gui_settings)
    , objects_ref(editor_ctx.objects_ref)
    , window_flags(window_flags)
{
}

void ObjectsRefWindow::show() const
{
    bool& show_objects_ref = gui_settings.show_objects_ref;

    if (!show_objects_ref)
    {
        return;
    }

    ImGui::Begin("objects.ref", &show_objects_ref, window_flags);

    static char search_buf[SEARCH_BUF_SIZE] = "";
    ImGui::InputText("label", search_buf, SEARCH_BUF_SIZE);

    std::string search_lower = search_buf;
    std::transform(search_lower.begin(), search_lower.end(),
                   search_lower.begin(), ::tolower);

    if (ImGui::BeginTable("ObjectsRefTable", 2,
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                          ImGuiTableFlags_SizingFixedFit))
    {
        // for (const auto& [label, ref] : objects_ref)
        // {
        //     std::string label_lower = label;
        //     std::transform(label.begin(), label.end(),
        //                    label.begin(), ::tolower);
        //     if (search_buf[0] != '\0' &&
        //         label_lower.find(search_lower) == std::string::npos)
        //     {
        //         continue;
        //     }

        //     ImGui::TableNextRow();
        //     ImGui::TableNextColumn();

        //     if (ImGui::Button(label.c_str()))
        //     {
        //         const auto object = RouteObject::create()
        //     }
        // }

        ImGui::EndTable();
    }

    ImGui::End();
}
