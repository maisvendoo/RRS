#include    <imgui-widgets-handler.h>
#include    <QStringList>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ImGuiWidgetsHandler::ImGuiWidgetsHandler()
{
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../fonts/arial.ttf",
                                 font_size,
                                 NULL,
                                 io.Fonts->GetGlyphRangesCyrillic());
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
int getLinesCount(const QString &str)
{
    QStringList lines = str.split('\n');

    return lines.count();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::showDebugLog()
{
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 content_size = io.DisplaySize;

    float h = 1.2 * font_size * getLinesCount(debugMsg);

    ImGui::SetNextWindowSize(ImVec2(content_size.x, h));
    ImGui::SetNextWindowPos(ImVec2(0, content_size.y - h));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Консоль отладки", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::Text(u8"%s", debugMsg.toStdString().c_str());
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::showUncontrolledState()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Сосотояние управления", &open_ptr, window_flags);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::Text(u8"%s", "Нажмите Enter для управления данной ПЕ");
    ImGui::PopStyleColor();
    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::setStatusBar(const QString &msg)
{
    debugMsg = msg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ImGuiWidgetsHandler::receiveControlledState(bool state)
{
    is_controlled = state;
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

    is_modified_key = io.KeysDown[ImGuiKey_LeftShift] || io.KeysDown[ImGuiKey_RightShift]
                   || io.KeysDown[ImGuiKey_LeftCtrl] || io.KeysDown[ImGuiKey_RightCtrl]
                   || io.KeysDown[ImGuiKey_LeftAlt] || io.KeysDown[ImGuiKey_RightAlt];

    if (io.KeysDown[ImGuiKey_Escape] && !is_Esc)
    {
        is_show_quit_dialog = !is_show_quit_dialog;
    }

    is_Esc = io.KeysDown[ImGuiKey_Escape];

    if (is_show_quit_dialog)
        showQuitDialog(is_show_quit_dialog);

    if (io.KeysDown[ImGuiKey_F1] && !is_F1 && !is_modified_key)
    {
        is_show_debug_log = !is_show_debug_log;
    }

    is_F1 = io.KeysDown[ImGuiKey_F1];

    if (is_show_debug_log)
        showDebugLog();

    if (!is_controlled)
        showUncontrolledState();
}
