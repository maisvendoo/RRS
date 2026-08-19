#include <TrainProfileHintWidget.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainProfileHintWidget::TrainProfileHintWidget(TrainProfileHintWidgetParams *params)
    : _params(params)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainProfileHintWidget::show(float top_y, float bottom_y)
{
    if (!_params || !_params->is_visible)
    {
        return;
    }

    const ImVec2 display_size = ImGui::GetIO().DisplaySize;
    if (bottom_y <= top_y)
    {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(0.0f, top_y));
    ImGui::SetNextWindowSize(ImVec2(display_size.x, bottom_y - top_y));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoInputs;

    bool open_ptr = true;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin(u8"Профиль пути поезда", &open_ptr, window_flags);
    ImGui::PopStyleColor();

    ImGui::End();
}