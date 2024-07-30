#include    <imgui-widgets-handler.h>
#include    <QApplication>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::showQuitDialog(bool &is_show)
{
    int w = 300;
    int h = 150;

    int cx = w / 2;
    int cy = h / 2;

    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiIO &io = ImGui::GetIO();

    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2( (content_size.x - w) / 2, (content_size.y - h) / 2));

    ImGui::Begin("A you realy want ot quit?");

    int bw = w / 4;
    int bh = h / 4;

    ImGui::SetCursorPos(ImVec2(cx - 3 * bw / 2, cy - bh / 2));
    if (ImGui::Button("YES", ImVec2(bw, bh)))
    {
        ImGui::SetCursorPos(ImVec2(cx, cy));
        exit(0);
    }

    ImGui::SetCursorPos(ImVec2(cx + bw / 2, cy - bh / 2));
    if (ImGui::Button("NO", ImVec2(bw, bh)))
    {
        ImGui::SetCursorPos(ImVec2(cx, cy));
        is_show = false;
    }

    ImGui::End();

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ImGuiWidgetsHandler::handle(const osgGA::GUIEventAdapter &ea,
                                 osgGA::GUIActionAdapter &aa)
{
    return OSGImGuiHandler::handle(ea, aa);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::drawUI()
{
    ImGuiIO &io = ImGui::GetIO();

    if (io.KeysDown[ImGuiKey_Escape])
    {
        is_show_quit_dialog = true;
    }

    if (is_show_quit_dialog)
        showQuitDialog(is_show_quit_dialog);
}
