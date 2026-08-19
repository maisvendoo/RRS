#include <TrainProfileHintWidget.h>

#include <algorithm>
#include <cmath>
#include <vector>

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

    // Получаем профиль пути текущего поезда
    _profile_valid = false;
    if (_params->vehicles_handler)
    {
        const int train_id = _params->vehicles_handler->getCurrentTrainIndex();
        if (train_id >= 0)
        {
            _profile_valid = _params->vehicles_handler->getTrainProfile(train_id, _profile);
        }
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

    if (_profile_valid && !_profile.profile.empty())
    {
        drawProfile();
    }

    ImGui::End();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainProfileHintWidget::drawProfile() const
{
    const std::vector<simulator_train_profile_point_t>& points = _profile.profile;
    if (points.size() < 2)
        return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 win_pos = ImGui::GetWindowPos();
    const ImVec2 win_size = ImGui::GetWindowSize();

    const float pad = 10.0f;
    const float x0 = win_pos.x + pad;
    const float x1 = win_pos.x + win_size.x - pad;
    const float y0 = win_pos.y + pad;
    const float y1 = win_pos.y + win_size.y - pad;
    const float cx = (x0 + x1) * 0.5f;

    // Отсчётная высота - середина поезда (distance = 0), линейная интерполяция
    float origin_elev = 0.0f;
    {
        size_t i = 0;
        while (i < points.size() && points[i].distance < 0.0f)
            ++i;
        if (i == 0)
        {
            origin_elev = points.front().elevation;
        }
        else if (i >= points.size())
        {
            origin_elev = points.back().elevation;
        }
        else
        {
            const float d0 = points[i - 1].distance;
            const float d1 = points[i].distance;
            const float k = (d1 - d0) > 1e-9f ? (0.0f - d0) / (d1 - d0) : 0.0f;
            origin_elev = points[i - 1].elevation + k * (points[i].elevation - points[i - 1].elevation);
        }
    }

    // Диапазоны по вертикали (относительная высота)
    float rel_min = points.front().elevation - origin_elev;
    float rel_max = rel_min;
    for (const auto& p : points)
    {
        const float rel = p.elevation - origin_elev;
        if (rel < rel_min)
            rel_min = rel;
        if (rel > rel_max)
            rel_max = rel;
    }

    float rel_span = rel_max - rel_min;
    if (rel_span < 0.5f)
    {
        rel_min -= (0.5f - rel_span) * 0.5f;
        rel_max += (0.5f - rel_span) * 0.5f;
        rel_span = rel_max - rel_min;
    }
    rel_min -= 0.05f * rel_span;
    rel_max += 0.05f * rel_span;

    // Горизонтальный масштаб: центр (distance = 0) - середина окна, весь профиль помещается
    const float d_min = points.front().distance;
    const float d_max = points.back().distance;
    const float half_extent = std::max(std::abs(d_min), std::abs(d_max));
    const float x_scale = (half_extent > 1e-6f) ? (x1 - x0) / (2.0f * half_extent) : 1.0f;
    const float y_scale = (y1 - y0) / rel_span;

    const auto map_x = [&](float d) { return cx + d * x_scale; };
    const auto map_y = [&](float rel) { return y1 - (rel - rel_min) * y_scale; };

    // Базовая линия на уровне середины поезда (rel = 0)
    if (0.0f >= rel_min && 0.0f <= rel_max)
    {
        const float y_base = map_y(0.0f);
        draw_list->AddLine(ImVec2(x0, y_base), ImVec2(x1, y_base),
                           IM_COL32(128, 128, 128, 255), 1.0f);
    }

    // Кривая профиля
    std::vector<ImVec2> poly;
    poly.reserve(points.size());
    for (const auto& p : points)
    {
        poly.emplace_back(map_x(p.distance), map_y(p.elevation - origin_elev));
    }
    draw_list->AddPolyline(poly.data(), static_cast<int>(poly.size()),
                           IM_COL32(0x00, 0x66, 0xCC, 255), 0, 2.0f);
}