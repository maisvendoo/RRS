#include    <imgui-widgets-handler.h>
#include    <QApplication>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ImGuiWidgetsHandler::ImGuiWidgetsHandler()
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../fonts/arial.ttf", 20, NULL, io.Fonts->GetGlyphRangesCyrillic());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::showQuitDialog(bool &is_show)
{
    int w = 400;
    int h = 150;

    int cx = w / 2;
    int cy = h / 2;

    ImGui::SetNextWindowSize(ImVec2(w, h));

    ImGuiIO &io = ImGui::GetIO();

    ImVec2 content_size = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2( (content_size.x - w) / 2, (content_size.y - h) / 2));

    ImGui::Begin(u8"Вы действительно хотите выйти?");

    int bw = w / 4;
    int bh = h / 4;

    ImGui::SetCursorPos(ImVec2(cx - 3 * bw / 2, cy - bh / 2));
    if (ImGui::Button(u8"Да", ImVec2(bw, bh)))
    {
        ImGui::SetCursorPos(ImVec2(cx, cy));
        exit(0);
    }

    ImGui::SetCursorPos(ImVec2(cx + bw / 2, cy - bh / 2));
    if (ImGui::Button(u8"Нет", ImVec2(bw, bh)))
    {
        ImGui::SetCursorPos(ImVec2(cx, cy));
        is_show = false;
    }

    ImGui::End();

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::showDebugLog()
{
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 content_size = io.DisplaySize;

    int h = 200;

    ImGui::SetNextWindowSize(ImVec2(content_size.x, h));
    ImGui::SetNextWindowPos(ImVec2(0, content_size.y - h));

    ImGui::Begin(u8"Консоль отладки");
    ImGui::Text(u8"%s", debugMsg.toStdString().c_str());
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::setStatusBar(const QString &msg)
{
    debugMsg = msg;
}



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

    if (io.KeysDown[ImGuiKey_F1] && !is_F1)
    {
        is_show_debug_log = !is_show_debug_log;
    }

    is_F1 = io.KeysDown[ImGuiKey_F1];

    if (is_show_debug_log)
        showDebugLog();
}
