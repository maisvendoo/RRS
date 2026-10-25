#include "editor/gui/ObjectsRefWindow.h"

#include "editor/EditorContext.h"

#include <vsgImGui/imgui.h>

#include <algorithm>
#include <string>

#define SEARCH_BUF_SIZE 256

ObjectsRefWindow::ObjectsRefWindow(EditorContext& editor_ctx,
    ImGuiWindowFlags window_flags)
    : gui_settings(editor_ctx.gui_settings)
    , window_flags(window_flags)
{
}

void ObjectsRefWindow::show() const
{
    if (!gui_settings.show_objects_ref)
        return;

    ImGui::Begin("objects.ref", &gui_settings.show_objects_ref, window_flags);

    static char search_buf[SEARCH_BUF_SIZE] = "";
    ImGui::InputText("label", search_buf, SEARCH_BUF_SIZE);

    std::string search_lower = search_buf;
    std::transform(search_lower.begin(), search_lower.end(),
        search_lower.begin(), ::tolower);

    if (ImGui::BeginTable("ObjectsRefTable", 2, ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
    {
        // for (const auto& [label, ref] : )

        ImGui::EndTable();
    }

    ImGui::End();
}
