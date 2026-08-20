#include <TrainProfileHintWidget.h>

#include <algorithm>
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
void TrainProfileHintWidget::show(float top_y, float bottom_y,
                                  float left_inset, float right_inset)
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

    ImGui::SetNextWindowPos(ImVec2(left_inset, top_y));
    ImGui::SetNextWindowSize(ImVec2(display_size.x - left_inset - right_inset,
                                    bottom_y - top_y));

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
// Сборка меток координатной сетки. Километраж берётся напрямую из данных
// траектории (в .traj каждой дуговой координате соответствует свой километраж).
// Участки, где километраж отсутствует (нули вместо значений), линейно
// продолжаются от последнего реального значения до следующего - километраж
// может меняться скачком, это нормально. Для каждого значения, кратного
// grid_step, попадающего в интервал между концами сегмента, дистанция
// вычисляется линейной интерполяцией. Если во всём профиле километраж
// отсутствует (нет ни одного реального значения), метки расставляются
// равномерно с шагом grid_step по дистанции
//------------------------------------------------------------------------------
static void collectGridMarks(const std::vector<simulator_train_profile_point_t>& points,
                             float grid_step,
                             std::vector<std::pair<float, float>>& out)
{
    bool any_real_km = false;
    for (const auto& p : points)
    {
        if (p.railway_coord > 0.5f)
        {
            any_real_km = true;
            break;
        }
    }

    // Километраж отсутствует во всём профиле - метки по дистанции
    if (!any_real_km)
    {
        for (size_t i = 1; i < points.size(); ++i)
        {
            const float d0 = points[i - 1].distance;
            const float d1 = points[i].distance;
            const float span = d1 - d0;
            if (span <= 0.0f)
                continue;

            const int k_first = static_cast<int>(std::ceil(d0 / grid_step));
            const int k_last = static_cast<int>(std::floor(d1 / grid_step));
            for (int k = k_first; k <= k_last; ++k)
                out.emplace_back(0.0f, k * grid_step);
        }
        return;
    }

    // Продлеваем километраж через участки с обрывом данных (нули):
    // линейная интерполяция между реальными значениями по обе стороны
    std::vector<simulator_train_profile_point_t> bridged = points;

    size_t i = 0;
    while (i < bridged.size())
    {
        if (bridged[i].railway_coord > 0.5f)
        {
            ++i;
            continue;
        }

        size_t start = i;
        size_t end = i;
        while (end < bridged.size() && bridged[end].railway_coord <= 0.5f)
            ++end;

        if (start > 0 && end < bridged.size())
        {
            const float rc_a = bridged[start - 1].railway_coord;
            const float rc_b = bridged[end].railway_coord;
            const float d_a = bridged[start - 1].distance;
            const float d_b = bridged[end].distance;
            const float span = d_b - d_a;
            for (size_t j = start; j < end; ++j)
            {
                const float t = (span > 1e-6f) ? (bridged[j].distance - d_a) / span : 0.0f;
                bridged[j].railway_coord = rc_a + t * (rc_b - rc_a);
            }
        }

        i = end;
    }

    for (size_t i = 1; i < bridged.size(); ++i)
    {
        const float rc0 = bridged[i - 1].railway_coord;
        const float rc1 = bridged[i].railway_coord;
        const float d0 = bridged[i - 1].distance;
        const float d1 = bridged[i].distance;

        // Километраж всё ещё отсутствует (нули по краям профиля)
        if (rc0 <= 0.5f || rc1 <= 0.5f)
            continue;

        const float lo = std::min(rc0, rc1);
        const float hi = std::max(rc0, rc1);
        if (hi - lo < 1e-6f)
            continue;

        const int k_first = static_cast<int>(std::ceil(lo / grid_step));
        const int k_last = static_cast<int>(std::floor(hi / grid_step));
        for (int k = k_first; k <= k_last; ++k)
        {
            const float rc = k * grid_step;
            const float t = (rc - rc0) / (rc1 - rc0);
            out.emplace_back(rc, d0 + t * (d1 - d0));
        }
    }
}

//------------------------------------------------------------------------------
// Формат подписи метки сетки: "километр пикет" (пикет = 100 м).
// Для ровного километра пикет не выводится
//------------------------------------------------------------------------------
static QString formatKmPiket(float railway_coord)
{
    int km = static_cast<int>(railway_coord) / 1000;
    int piket = (static_cast<int>(railway_coord) % 1000) / 100;
    if (piket == 0)
        return QString("%1км").arg(km);
    return QString("%1км %2пк").arg(km).arg(piket);
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

    // Запрошенный диапазон отображения: настраиваемый клиентом, не более того,
    // что запрошено у сервера (серверный диапазон может быть меньше, если
    // подписка уже передала меньшие дальности)
    const float cfg_backward = std::max(_params->backward_m, 0.0f);
    const float cfg_forward = std::max(_params->forward_m, 0.0f);
    const float req_backward = std::min(cfg_backward, std::max(_profile.backward_requested, 0.0f));
    const float req_forward = std::min(cfg_forward, std::max(_profile.forward_requested, 0.0f));

    // Отсчётная высота - середина поезда (distance = 0)
    plot.origin_elev = elevationAt(0.0f, points);

    // Отбор точек профиля в пределах запрошенного диапазона
    std::vector<simulator_train_profile_point_t> clipped;
    clipped.reserve(points.size());
    for (const auto& p : points)
    {
        if (p.distance >= -req_backward && p.distance <= req_forward)
            clipped.push_back(p);
    }
    if (clipped.size() < 2)
        return;

    // Диапазоны по вертикали (относительная высота)
    plot.rel_min = clipped.front().elevation - plot.origin_elev;
    plot.rel_max = plot.rel_min;
    for (const auto& p : clipped)
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
    const float half_extent = std::max(req_backward, req_forward);
    plot.x_scale = (half_extent > 1e-6f) ? (x1 - x0) / (2.0f * half_extent) : 1.0f;

    // Вертикальный масштаб: кривая занимает половину высоты окна, центр поезда - в центре
    const float band_height = (y1 - y0) * 0.5f;
    plot.y_scale = band_height / rel_span;

    // Координатная сетка: вертикальные метки на всю высоту виджета,
    // привязанные к километровым/пикетным столбам. Километраж берётся
    // напрямую из данных траектории (профиль заполнен railway_coord из .traj),
    // поэтому подписи и позиции меток - реальный километраж пути
    const float grid_step = 500.0f;
    if (grid_step > 1e-6f)
    {
        const ImU32 grid_col = IM_COL32(90, 90, 90, 150);
        const ImU32 label_col = IM_COL32(190, 190, 190, 220);
        const ImFont* font = ImGui::GetFont();
        const float label_y = y1 - font->FontSize;

        std::vector<std::pair<float, float>> marks;
        collectGridMarks(points, grid_step, marks);

        for (const auto& mark : marks)
        {
            const float rc = mark.first;
            const float d = mark.second;

            const float gx = plot.map_x(d);

            draw_list->AddLine(ImVec2(gx, y0), ImVec2(gx, y1), grid_col, 1.0f);

            // Если километр и пикет равны 0 - подпись не рисуем, отметку оставляем
            if (rc < 100.0f)
                continue;

            const QString label = formatKmPiket(rc);
            const float text_w = ImGui::CalcTextSize(label.toStdString().c_str()).x;
            const float tx = std::max(x0, std::min(gx - text_w * 0.5f, x1 - text_w));
            draw_list->AddText(ImVec2(tx, label_y), label_col, label.toStdString().c_str());
        }
    }

    // Базовая линия на уровне середины поезда (rel = 0)
    if (0.0f >= plot.rel_min && 0.0f <= plot.rel_max)
    {
        const float y_base = plot.map_y(0.0f);
        draw_list->AddLine(ImVec2(x0, y_base), ImVec2(x1, y_base),
                           IM_COL32(128, 128, 128, 255), 1.0f);
    }

    // Кривая профиля
    std::vector<ImVec2> poly;
    poly.reserve(clipped.size());
    for (const auto& p : clipped)
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

    // Запрошенный диапазон отображения (как в drawProfile)
    const float cfg_backward = std::max(_params->backward_m, 0.0f);
    const float cfg_forward = std::max(_params->forward_m, 0.0f);
    const float req_backward = std::min(cfg_backward, std::max(_profile.backward_requested, 0.0f));
    const float req_forward = std::min(cfg_forward, std::max(_profile.forward_requested, 0.0f));

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
        // видимый промежуток, а общая длина поезда не менялась.
        // Интервал обрезаем по запрошенному диапазону отображения
        const float span = vehicle.end_distance - vehicle.begin_distance;
        const float gap = span * 0.1f;
        float d0 = vehicle.begin_distance + gap * 0.5f;
        float d1 = vehicle.end_distance - gap * 0.5f;
        d0 = std::max(d0, -req_backward);
        d1 = std::min(d1, req_forward);
        if (d1 <= d0)
            continue;
        const float rel0 = elevationAt(d0, _profile.profile) - plot.origin_elev;
        const float rel1 = elevationAt(d1, _profile.profile) - plot.origin_elev;

        draw_list->AddLine(ImVec2(plot.map_x(d0), plot.map_y(rel0)),
                           ImVec2(plot.map_x(d1), plot.map_y(rel1)),
                           color, 5.0f);
    }
}
