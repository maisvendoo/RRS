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
// Высота пути на заданной дистанции от середины поезда (линейная интерполяция)
//------------------------------------------------------------------------------
static float elevationAt(float distance,
                         const std::vector<simulator_train_profile_point_t>& points)
{
    if (points.empty())
        return 0.0f;

    size_t i = 0;
    while (i < points.size() && points[i].distance < distance)
        ++i;

    if (i == 0)
        return points.front().elevation;
    if (i >= points.size())
        return points.back().elevation;

    const float d0 = points[i - 1].distance;
    const float d1 = points[i].distance;
    const float k = (d1 - d0) > 1e-9f ? (distance - d0) / (d1 - d0) : 0.0f;
    return points[i - 1].elevation + k * (points[i].elevation - points[i - 1].elevation);
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

    PlotTransform plot;
    plot.cx = (x0 + x1) * 0.5f;
    plot.cy = (y0 + y1) * 0.5f;
    plot.x0 = x0;
    plot.x1 = x1;
    plot.y0 = y0;
    plot.y1 = y1;

    // Отсчётная высота - середина поезда (distance = 0)
    plot.origin_elev = elevationAt(0.0f, points);

    // Диапазоны по вертикали (относительная высота)
    plot.rel_min = points.front().elevation - plot.origin_elev;
    plot.rel_max = plot.rel_min;
    for (const auto& p : points)
    {
        const float rel = p.elevation - plot.origin_elev;
        if (rel < plot.rel_min)
            plot.rel_min = rel;
        if (rel > plot.rel_max)
            plot.rel_max = rel;
    }

    float rel_span = plot.rel_max - plot.rel_min;
    if (rel_span < 0.5f)
    {
        plot.rel_min -= (0.5f - rel_span) * 0.5f;
        plot.rel_max += (0.5f - rel_span) * 0.5f;
        rel_span = plot.rel_max - plot.rel_min;
    }
    plot.rel_min -= 0.05f * rel_span;
    plot.rel_max += 0.05f * rel_span;
    rel_span = plot.rel_max - plot.rel_min;

    // Горизонтальный масштаб: центр (distance = 0) - середина окна,
    // масштаб соответствует запрошенному диапазону вперёд/назад даже если
    // обход упёрся в непроходимую стрелку
    const float req_backward = std::max(_profile.backward_requested, 0.0f);
    const float req_forward = std::max(_profile.forward_requested, 0.0f);
    const float half_extent = std::max(req_backward, req_forward);
    plot.x_scale = (half_extent > 1e-6f) ? (x1 - x0) / (2.0f * half_extent) : 1.0f;

    // Вертикальный масштаб: кривая занимает половину высоты окна, центр поезда - в центре
    const float band_height = (y1 - y0) * 0.5f;
    plot.y_scale = band_height / rel_span;

    // Базовая линия на уровне середины поезда (rel = 0)
    if (0.0f >= plot.rel_min && 0.0f <= plot.rel_max)
    {
        const float y_base = plot.map_y(0.0f);
        draw_list->AddLine(ImVec2(x0, y_base), ImVec2(x1, y_base),
                           IM_COL32(128, 128, 128, 255), 1.0f);
    }

    // Кривая профиля
    std::vector<ImVec2> poly;
    poly.reserve(points.size());
    for (const auto& p : points)
    {
        poly.emplace_back(plot.map_x(p.distance),
                          plot.map_y(p.elevation - plot.origin_elev));
    }
    draw_list->AddPolyline(poly.data(), static_cast<int>(poly.size()),
                           IM_COL32(0x00, 0x66, 0xCC, 255), 0, 2.0f);

    drawTrain(plot);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainProfileHintWidget::drawTrain(const PlotTransform& plot) const
{
    if (!_params->vehicles_handler)
        return;

    const std::vector<simulator_train_profile_vehicle_t>& vehicles = _profile.vehicles;
    if (vehicles.empty())
        return;

    const int current_vehicle = _params->vehicles_handler->getCurrentVehicleIndex();
    const int controlled_vehicle = _params->vehicles_handler->getControlledVehicleIndex();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const ImU32 color_uncontrolled = IM_COL32(64, 128, 0, 255);
    const ImU32 color_current = IM_COL32(192, 192, 0, 255);
    const ImU32 color_controlled = IM_COL32(192, 64, 64, 255);

    for (const auto& vehicle : vehicles)
    {
        const int model_index = vehicle.vehicle_id;

        ImU32 color = color_uncontrolled;
        if (model_index == controlled_vehicle)
            color = color_controlled;
        else if (model_index == current_vehicle)
            color = color_current;

        // Вагон рисуется сегментом вдоль линии профиля на занимаемый интервал;
        // длину рисовки уменьшаем на ширину зазора, чтобы между вагонами был
        // видимый промежуток, а общая длина поезда не менялась
        const float span = vehicle.end_distance - vehicle.begin_distance;
        const float gap = span * 0.1f;
        const float d0 = vehicle.begin_distance + gap * 0.5f;
        const float d1 = vehicle.end_distance - gap * 0.5f;
        const float rel0 = elevationAt(d0, _profile.profile) - plot.origin_elev;
        const float rel1 = elevationAt(d1, _profile.profile) - plot.origin_elev;

        draw_list->AddLine(ImVec2(plot.map_x(d0), plot.map_y(rel0)),
                           ImVec2(plot.map_x(d1), plot.map_y(rel1)),
                           color, 5.0f);
    }
}