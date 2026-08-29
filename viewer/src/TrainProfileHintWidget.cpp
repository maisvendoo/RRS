#include <TrainProfileHintWidget.h>

#include <TrafficLightsHandler.h>
#include <TrafficLight.h>

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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, _params->hud_background);
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
// Число линз светофора по суффиксу модели (как в tools/route-map).
// Возвращает 0 для неизвестного типа - такой светофор не рисуется
//------------------------------------------------------------------------------
static int signalLensCount(const QString& model)
{
    if (model.isEmpty() || model.startsWith("empty_"))
        return 0;
    if (model.endsWith("line"))
        return 3;
    if (model.endsWith("entr") || model.endsWith("rout"))
        return 5;
    if (model.endsWith("exit"))
        return 4;
    if (model.endsWith("shnt"))
        return 2;
    return 0;
}

//------------------------------------------------------------------------------
// Высота мачты светофора на виджете, px (от высоты профиля до верхней линзы).
// Литер рисуется над мачтой отдельно (см. drawSignals)
//------------------------------------------------------------------------------
static float signalHeightPx(int lens_count)
{
    const float lens_r = 4.0f;
    const float lens_gap = 2.0f * lens_r;
    return (lens_count + 1) * lens_gap + 8.0f;
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
            {
                const float rc = 0.0f;
                const float d = k * grid_step;
                if (!out.empty() && std::abs(out.back().second - d) < 0.5f)
                    continue;
                out.emplace_back(rc, d);
            }
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
            // Дедупликация: не добавляем метку, если последняя добавленная
            // имеет тот же railway_coord (с точностью до шага)
            if (!out.empty() && std::abs(out.back().first - rc) < 0.5f)
                continue;
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

    // Светофоры рисуются вертикально вверх от высоты профиля (мачта, линзы,
    // литер). Чтобы они помещались в окно, верхний предел диапазона расширяем
    // на высоту самого высокого светофора. Высота в метрах зависит от
    // вертикального масштаба, поэтому уточняем итеративно (3 прохода)
    if (_params->traffic_lights_handler)
    {
        for (int pass = 0; pass < 3; ++pass)
        {
            float rel_top_max = plot.rel_max;
            for (const auto& sig : _profile.signal_list)
            {
                if (sig.distance < -req_backward || sig.distance > req_forward)
                    continue;

                TrafficLight* traffic_light = _params->traffic_lights_handler
                    ->findSignal(sig.connector_name, sig.signal_dir);
                if (traffic_light == nullptr)
                    continue;

                const int lens_count = signalLensCount(traffic_light->getModelName());
                if (lens_count <= 0)
                    continue;

                const float height_px = signalHeightPx(lens_count)
                                        + (traffic_light->getLetter().isEmpty() ? 0.0f : 16.0f);
                const float rel_sig = elevationAt(sig.distance, _profile.profile)
                                      - plot.origin_elev;
                const float rel_top = rel_sig + height_px / plot.y_scale;
                rel_top_max = std::max(rel_top_max, rel_top);
            }

            if (rel_top_max <= plot.rel_max)
                break;

            plot.rel_max = rel_top_max;
            rel_span = plot.rel_max - plot.rel_min;
            plot.y_scale = band_height / rel_span;
        }
    }

    // Координатная сетка: вертикальные метки на всю высоту виджета,
    // привязанные к километровым/пикетным столбам. Километраж берётся
    // напрямую из данных траектории (профиль заполнен railway_coord из .traj),
    // поэтому подписи и позиции меток - реальный километраж пути
    const float grid_step = 500.0f;
    if (grid_step > 1e-6f)
    {
        const ImU32 grid_col = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_grid);
        const ImU32 label_col = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_grid_label);
        const ImFont* font = ImGui::GetFont();
        const float label_y = y1 - font->LegacySize;

        std::vector<std::pair<float, float>> marks;
        collectGridMarks(points, grid_step, marks);

        for (const auto& mark : marks)
        {
            const float rc = mark.first;
            const float d = mark.second;

            const float gx = plot.map_x(d);

            draw_list->AddLine(ImVec2(gx, y0), ImVec2(gx, y1), grid_col, 1.0f);

            // Текстовую подпись рисуем только на целых километрах; на пикетах
            // (в том числе пятом, шаг сетки 500 м) оставляем только вертикальную линию
            if (rc < 100.0f)
                continue;

            const int piket = (static_cast<int>(rc) % 1000) / 100;
            if (piket != 0)
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
                           ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_baseline), 1.0f);
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
                           ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_curve), 0, 2.0f);

    drawSpeedLimits(plot);

    drawSignals(plot);

    drawTrain(plot);

    drawTrainNames(plot);

    drawStations(plot);
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

    const ImU32 color_uncontrolled = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_uncontrolled);
    const ImU32 color_current = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_current);
    const ImU32 color_controlled = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_controlled);

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

//------------------------------------------------------------------------------
// Отрисовка имён поездов над их составами на профиле. Имена берутся из
// данных о поездах вьювера (VehiclesHandler::getTrainsInfo), где поезд задан
// диапазоном model-index своих ПЕ. ПЕ профиля группируются по принадлежности
// к поезду, интервалы соседних ПЕ одного поезда сливаются, имя рисуется над
// центром видимой части состава
//------------------------------------------------------------------------------
void TrainProfileHintWidget::drawTrainNames(const PlotTransform& plot) const
{
    if (!_params->vehicles_handler)
        return;

    const auto& trains_info = _params->vehicles_handler->getTrainsInfo();
    if (trains_info.empty())
        return;

    const std::vector<simulator_train_profile_vehicle_t>& vehicles = _profile.vehicles;
    if (vehicles.empty())
        return;

    // Запрошенный диапазон отображения (как в drawTrain)
    const float cfg_backward = std::max(_params->backward_m, 0.0f);
    const float cfg_forward = std::max(_params->forward_m, 0.0f);
    const float req_backward = std::min(cfg_backward, std::max(_profile.backward_requested, 0.0f));
    const float req_forward = std::min(cfg_forward, std::max(_profile.forward_requested, 0.0f));

    // Диапазоны model-index ПЕ поездов из данных вьювера
    struct train_range_t
    {
        int begin_id = 0;
        int end_id = 0;
        QString name;
    };
    std::vector<train_range_t> ranges;
    ranges.reserve(trains_info.size());
    for (const auto& info : trains_info)
    {
        train_range_t range;
        range.begin_id = std::min(info.first_vehicle_id, info.last_vehicle_id);
        range.end_id = std::max(info.first_vehicle_id, info.last_vehicle_id);
        range.name = info.train_name;
        ranges.push_back(range);
    }

    // Группировка ПЕ профиля по поездам: для каждой ПЕ находим поезд по
    // model-index и объединяем в общий интервал состава. Все интервалы одного
    // поезда сливаются в одну группу независимо от разрывов между ними
    // (например, при пересечении точкой отсчёта профиля), чтобы имя поезда
    // рисовалось один раз над центром всего состава
    struct train_group_t
    {
        int train_index = -1;
        QString name;
        float begin = 0.0f;
        float end = 0.0f;
    };
    std::vector<train_group_t> groups;

    for (const auto& vehicle : vehicles)
    {
        const int model_index = vehicle.vehicle_id;

        int matched_index = -1;
        for (size_t i = 0; i < ranges.size(); ++i)
        {
            if (model_index >= ranges[i].begin_id && model_index <= ranges[i].end_id)
            {
                matched_index = static_cast<int>(i);
                break;
            }
        }
        if (matched_index < 0 || ranges[matched_index].name.isEmpty())
            continue;

        bool merged = false;
        for (auto& group : groups)
        {
            if (group.train_index == matched_index)
            {
                group.begin = std::min(group.begin, vehicle.begin_distance);
                group.end = std::max(group.end, vehicle.end_distance);
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            train_group_t group;
            group.train_index = matched_index;
            group.name = ranges[matched_index].name;
            group.begin = vehicle.begin_distance;
            group.end = vehicle.end_distance;
            groups.push_back(group);
        }
    }

    if (groups.empty())
        return;

    const int current_train_index = _params->vehicles_handler->getCurrentTrainIndex();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const float text_offset_y = 12.0f;  // отступ подписи над линией профиля

    for (const auto& group : groups)
    {
        // Обрезаем интервал поезда по видимой области
        const float g0 = std::max(group.begin, -req_backward);
        const float g1 = std::min(group.end, req_forward);
        if (g1 <= g0)
            continue;

        // Центр видимой части состава и высота профиля в этой точке
        const float center = 0.5f * (g0 + g1);
        const float rel = elevationAt(center, _profile.profile) - plot.origin_elev;
        const float x = plot.map_x(center);
        const float y = plot.map_y(rel);

        // Текущий поезд выделяем жёлтым, остальные - белым
        const bool is_current = (group.train_index == current_train_index);
        const ImU32 color = is_current ? IM_COL32(255, 230, 0, 255)
                                       : IM_COL32(255, 255, 255, 255);

        const std::string label = group.name.toStdString();
        const ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
        const float tx = x - text_size.x * 0.5f;
        const float ty = y - text_offset_y - text_size.y;

        draw_list->AddText(ImVec2(tx, ty), color, label.c_str());
    }
}

//------------------------------------------------------------------------------
// Отрисовка названий станций на профиле. Станции приходят из обновления
// профиля (модель передаёт станции, попадающие на траектории профиля).
// Название рисуется под линией профиля без подложки, как литеры светофоров
//------------------------------------------------------------------------------
void TrainProfileHintWidget::drawStations(const PlotTransform& plot) const
{
    const std::vector<simulator_train_profile_station_t>& stations = _profile.stations;
    if (stations.empty())
        return;

    const float cfg_backward = std::max(_params->backward_m, 0.0f);
    const float cfg_forward = std::max(_params->forward_m, 0.0f);
    const float req_backward = std::min(cfg_backward, std::max(_profile.backward_requested, 0.0f));
    const float req_forward = std::min(cfg_forward, std::max(_profile.forward_requested, 0.0f));

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const float text_offset_y = 12.0f;  // отступ подписи под линией профиля
    const ImU32 text_col = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_station_text);  // голубой, как литеры светофоров

    for (const auto& station : stations)
    {
        if (station.distance < -req_backward || station.distance > req_forward)
            continue;

        const std::string label = station.name.toStdString();
        if (label.empty())
            continue;

        const float x = plot.map_x(station.distance);
        const float rel = elevationAt(station.distance, _profile.profile) - plot.origin_elev;
        const float y_base = plot.map_y(rel);

        const ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
        const float tx = x - text_size.x * 0.5f;
        const float ty = y_base + text_offset_y;

        draw_list->AddText(ImVec2(tx, ty), text_col, label.c_str());
    }
}

//------------------------------------------------------------------------------
// Отрисовка попутных светофоров на профиле. Данные о светофорах берутся из
// TrafficLightsHandler, куда они загружаются при старте вьювера (в том числе
// актуальное состояние линз, обновляемое по сети). Светофор рисуется
// вертикально: мачта от высоты профиля вверх, линзы снизу вверх, литер сверху
//------------------------------------------------------------------------------
void TrainProfileHintWidget::drawSignals(const PlotTransform& plot) const
{
    if (!_params->traffic_lights_handler)
        return;

    const std::vector<simulator_train_profile_signal_t>& sig_list = _profile.signal_list;
    if (sig_list.empty())
        return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const float cfg_backward = std::max(_params->backward_m, 0.0f);
    const float cfg_forward = std::max(_params->forward_m, 0.0f);
    const float req_backward = std::min(cfg_backward, std::max(_profile.backward_requested, 0.0f));
    const float req_forward = std::min(cfg_forward, std::max(_profile.forward_requested, 0.0f));

    const float lens_r = 4.0f;
    const float lens_gap = 2.0f * lens_r;
    const ImU32 mast_col = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_mast);
    const ImU32 off_col = IM_COL32(24, 24, 24, 255);

    // Состав и порядок линз для каждого типа светофора (снизу вверх)
    struct lens_spec_t
    {
        int lens;
        ImU32 lit_color;
    };

    const auto lens_color = [](int lens) -> ImU32
    {
        switch (lens)
        {
            case RED_LENS:           return IM_COL32(255, 0, 0, 255);
            case YELLOW_LENS:        return IM_COL32(255, 255, 0, 255);
            case GREEN_LENS:         return IM_COL32(0, 255, 0, 255);
            case WHITE_LENS:         return IM_COL32(255, 255, 196, 255);
            case BLUE_LENS:          return IM_COL32(0, 96, 255, 255);
            case BOTTOM_YELLOW_LENS: return IM_COL32(255, 255, 0, 255);
            default:                 return IM_COL32(255, 255, 196, 255);
        }
    };

    for (const auto& sig : sig_list)
    {
        if (sig.distance < -req_backward || sig.distance > req_forward)
            continue;

        TrafficLight* traffic_light = _params->traffic_lights_handler
            ->findSignal(sig.connector_name, sig.signal_dir);
        if (traffic_light == nullptr)
            continue;

        const QString model = traffic_light->getModelName();

        // Модели empty_* - заглушки для цепей АЛСН на неправильном пути,
        // не являются попутными сигналами и не должны отображаться
        if (model.isEmpty() || model.startsWith("empty_"))
            continue;

        const lens_state_t& lens = traffic_light->getLensState();

        // Набор линз и порядок их следования снизу вверх (как в tools/route-map)
        std::vector<lens_spec_t> spec;
        if (model.endsWith("line"))
            spec = {{RED_LENS, lens_color(RED_LENS)},
                    {GREEN_LENS, lens_color(GREEN_LENS)},
                    {YELLOW_LENS, lens_color(YELLOW_LENS)}};
        else if (model.endsWith("entr") || model.endsWith("rout"))
            spec = {{WHITE_LENS, lens_color(WHITE_LENS)},
                    {BOTTOM_YELLOW_LENS, lens_color(BOTTOM_YELLOW_LENS)},
                    {RED_LENS, lens_color(RED_LENS)},
                    {GREEN_LENS, lens_color(GREEN_LENS)},
                    {YELLOW_LENS, lens_color(YELLOW_LENS)}};
        else if (model.endsWith("exit"))
            spec = {{WHITE_LENS, lens_color(WHITE_LENS)},
                    {RED_LENS, lens_color(RED_LENS)},
                    {GREEN_LENS, lens_color(GREEN_LENS)},
                    {YELLOW_LENS, lens_color(YELLOW_LENS)}};
        else if (model.endsWith("shnt"))
            spec = {{BLUE_LENS, lens_color(BLUE_LENS)},
                    {WHITE_LENS, lens_color(WHITE_LENS)}};
        else
            continue;

        const float x = plot.map_x(sig.distance);
        const float rel = elevationAt(sig.distance, _profile.profile) - plot.origin_elev;
        const float y_base = plot.map_y(rel);

        const bool has_letter = !traffic_light->getLetter().isEmpty();
        const float mast_h = signalHeightPx(static_cast<int>(spec.size()));
        const float y_top = y_base - mast_h;

        // Мачта и перекладина
        draw_list->AddLine(ImVec2(x, y_base), ImVec2(x, y_top), mast_col, 1.5f);
        draw_list->AddLine(ImVec2(x - lens_r, y_base), ImVec2(x + lens_r, y_base), mast_col, 1.5f);

        // Линзы снизу вверх: горящая - ярким цветом, погашенная - тёмной
        for (size_t i = 0; i < spec.size(); ++i)
        {
            const float ly = y_base - (i + 1) * lens_gap;
            const bool lit = static_cast<size_t>(spec[i].lens) < lens.size()
                && lens[static_cast<size_t>(spec[i].lens)];
            const ImU32 col = lit ? spec[i].lit_color : off_col;
            draw_list->AddCircleFilled(ImVec2(x, ly), lens_r, col, 16);
        }

        // Литер над верхней линзой
        const QString letter = traffic_light->getLetter();
        if (!letter.isEmpty())
        {
            const std::string label = letter.toStdString();
            const float text_w = ImGui::CalcTextSize(label.c_str()).x;
            draw_list->AddText(ImVec2(x - text_w * 0.5f, y_top - 16.0f),
                               ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_signal_letter), label.c_str());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainProfileHintWidget::drawSpeedLimits(const PlotTransform& plot) const
{
    const std::vector<simulator_train_profile_speed_limit_t>& limits = _profile.speed_limits;
    if (limits.empty())
        return;

    const float cfg_backward = std::max(_params->backward_m, 0.0f);
    const float cfg_forward = std::max(_params->forward_m, 0.0f);
    const float req_backward = std::min(cfg_backward, std::max(_profile.backward_requested, 0.0f));
    const float req_forward = std::min(cfg_forward, std::max(_profile.forward_requested, 0.0f));

    // Базовая линия (y = 0) — уровень середины поезда
    const float y_base = (0.0f >= plot.rel_min && 0.0f <= plot.rel_max)
        ? plot.map_y(0.0f) : (plot.y0 + plot.y1) * 0.5f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const float zone_height = 20.0f;
    const float y_bottom = y_base + zone_height;
    const ImU32 col = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_speed_limit_border);
    const ImU32 fill_col = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_speed_limit_fill);
    const ImU32 text_col = ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_speed_limit_text);

    for (const auto& sl : limits)
    {
        const float d0 = sl.distance;
        const float d1 = sl.end_distance;
        if (d1 <= d0 || d1 < -req_backward || d0 > req_forward)
            continue;

        const float c0 = std::max(d0, -req_backward);
        const float c1 = std::min(d1, req_forward);
        if (c1 <= c0)
            continue;

        const float x0 = plot.map_x(c0);
        const float x1 = plot.map_x(c1);

        // Заливка + рамка
        draw_list->AddRectFilled(ImVec2(x0, y_base), ImVec2(x1, y_bottom), fill_col);
        draw_list->AddRect(ImVec2(x0, y_base), ImVec2(x1, y_bottom), col, 0.0f, 0, 1.5f);

        const float pad = 2.0f;

        // Подпись в начале зоны
        const std::string label = std::to_string(static_cast<int>(sl.speed_kmh));
        const ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
        const float tx = x0 + pad;
        const float ty = y_base + (zone_height - text_size.y) * 0.5f;

        // Белая непрозрачная подложка под текст (не выходит за границы ленты)
        const float bg_x0 = std::max(tx - pad, x0);
        const float bg_x1 = std::min(tx + text_size.x + pad, x1);
        const float bg_y0 = std::max(ty - pad, y_base);
        const float bg_y1 = std::min(ty + text_size.y + pad, y_bottom);
        if (bg_x1 > bg_x0 && bg_y1 > bg_y0)
            draw_list->AddRectFilled(ImVec2(bg_x0, bg_y0),
                                     ImVec2(bg_x1, bg_y1),
                                     ImGui::ColorConvertFloat4ToU32(_params->hud_train_profile_speed_limit_bg));

        // Жирный шрифт через наложение
        const float bold_off = 1.0f;
        draw_list->AddText(ImVec2(tx - bold_off, ty), text_col, label.c_str());
        draw_list->AddText(ImVec2(tx + bold_off, ty), text_col, label.c_str());
        draw_list->AddText(ImVec2(tx, ty - bold_off), text_col, label.c_str());
        draw_list->AddText(ImVec2(tx, ty + bold_off), text_col, label.c_str());
        draw_list->AddText(ImVec2(tx, ty), text_col, label.c_str());

        // Вертикальные линии от линии профиля до низа ленты (поверх подложки)
        const float rel0 = elevationAt(c0, _profile.profile) - plot.origin_elev;
        const float rel1 = elevationAt(c1, _profile.profile) - plot.origin_elev;
        draw_list->AddLine(ImVec2(x0, plot.map_y(rel0)), ImVec2(x0, y_bottom), text_col, 1.5f);
        draw_list->AddLine(ImVec2(x1, plot.map_y(rel1)), ImVec2(x1, y_bottom), text_col, 1.5f);
    }
}
