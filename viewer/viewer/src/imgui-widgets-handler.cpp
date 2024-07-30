#include    <imgui-widgets-handler.h>
#include    <QString>

void ImGuiWidgetsHandler::showQuitDialog()
{
    int w = 300;
    int h = 150;
    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiIO &io = ImGui::GetIO();

    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2( (content_size.x - w) / 2, (content_size.y - h) / 2));

    ImGui::Begin("A you realy want ot quit?");
    ImGui::Button("YES");
    ImGui::Button("NO");
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::drawUI()
{
    ImGuiIO &io = ImGui::GetIO();

    if (io.KeysDown[ImGuiKey_Escape])
    {
        is_quit = true;
    }

    if (is_quit)
        showQuitDialog();
}
