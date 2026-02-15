#include    <map-widget.h>
#include    <QPainter>
#include    <QMenu>
#include    <QTreeWidget>
#include    <QMouseEvent>
#include    <QWheelEvent>
#include    <connector.h>
#include    <switch.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MapWidget::~MapWidget()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::resize(int width, int height)
{
    QWidget::resize(width, height);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::setSwitchLength(double value)
{
    switch_length = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::setSignalRadius(double value)
{
    signal_radius = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::setSignalOffset(double value)
{
    signal_offset = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::calcCwitchCoords()
{
    switch_coords.clear();

    for (auto& sw : *conn_list)
    {
        switch_coord_t sc;

        dir_t dir_fwd = FWD;
        Trajectory *fwd_traj = sw->getNextTraj(dir_fwd);
        dir_t dir_bwd = BWD;
        Trajectory *bwd_traj = sw->getNextTraj(dir_bwd);
        if ((fwd_traj == nullptr) || (fwd_traj->getTracks().empty()))
        {
            if ((bwd_traj == nullptr || (bwd_traj->getTracks().empty())))
            {
                return;
            }
            else
            {
                const track_t& bwd_track = (dir_bwd == BWD) ? bwd_traj->getLastTrack() : bwd_traj->getFirstTrack();
                sc.center = (dir_bwd == BWD) ? bwd_track.end_point : bwd_track.begin_point;
                sc.orth = (dir_bwd == BWD) ? bwd_track.orth : -bwd_track.orth;
                sc.trav = (dir_bwd == BWD) ? bwd_track.trav : -bwd_track.trav;
            }
        }
        else
        {
            if ((bwd_traj == nullptr || (bwd_traj->getTracks().empty())))
            {
                const track_t& fwd_track = (dir_fwd == FWD) ? fwd_traj->getFirstTrack() : fwd_traj->getLastTrack();
                sc.center = (dir_fwd == FWD) ? fwd_track.begin_point : fwd_track.end_point;
                sc.orth = (dir_fwd == FWD) ? fwd_track.orth : -fwd_track.orth;
                sc.trav = (dir_fwd == FWD) ? fwd_track.trav : -fwd_track.trav;
            }
            else
            {
                const track_t& bwd_track = (dir_bwd == BWD) ? bwd_traj->getLastTrack() : bwd_traj->getFirstTrack();
                const track_t& fwd_track = (dir_fwd == FWD) ? fwd_traj->getFirstTrack() : fwd_traj->getLastTrack();
                sc.center = (  ((dir_bwd == BWD) ? bwd_track.end_point : bwd_track.begin_point)
                             + ((dir_fwd == FWD) ? fwd_track.begin_point : fwd_track.end_point)) * 0.5;
                sc.orth = normalize(
                      ((dir_bwd == BWD) ? bwd_track.orth : -bwd_track.orth)
                    + ((dir_fwd == FWD) ? fwd_track.orth : -fwd_track.orth));
                sc.trav = normalize(
                      ((dir_bwd == BWD) ? bwd_track.trav : -bwd_track.trav)
                    + ((dir_fwd == FWD) ? fwd_track.trav : -fwd_track.trav));
            }
        }
        switch_coords.insert(sw->getName(), sc);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::slotStationAtCenter(int idx)
{
    follow_player = false;

    if ((idx < 0) || (idx >= stations->size()))
        return;

    map_shift.setX(- stations->at(idx).pos_y * scale);
    map_shift.setY(- stations->at(idx).pos_x * scale);
    prev_map_shift = map_shift;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::slotPlayerAtCenter(int idx)
{
    follow_player = true;

    if ((idx < 0) || (idx >= players_data->current_vehicles.size()))
        return;

    follow_player_idx = idx;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::resetSwitchMenu()
{
    switch_menu = switch_menu_t();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if ((traj_list == nullptr) || (conn_list == nullptr))
    {
        return;
    }

    const double limit_dist = 4.0 + 2.0 * scale;
    double dist2_to_nearest_trajectory = limit_dist * limit_dist;
    double dist2_to_nearest_switch = limit_dist * limit_dist;
    nearest_trajectory = nullptr;
    nearest_switch = nullptr;
    nearest_switch_dir = 0;
    QPointF mouse_pos_current = mapFromGlobal(QCursor::pos());

    QPainter painter;
    painter.begin(this);

    for (auto& traj : *traj_list)
    {
        double distance2 = std::numeric_limits<double>::max();
        drawTrajectory(traj, painter, mouse_pos_current, distance2);

        if (dist2_to_nearest_trajectory > distance2)
        {
            dist2_to_nearest_trajectory = distance2;
            nearest_trajectory = traj;
        }
    }

    for (auto& conn : *conn_list)
    {
        double distance2 = std::numeric_limits<double>::max();
        std::int8_t switch_dir = 0;
        drawConnector(conn, painter, mouse_pos_current, distance2, switch_dir);

        if (dist2_to_nearest_switch > distance2)
        {
            dist2_to_nearest_switch = distance2;
            nearest_switch = conn;
            nearest_switch_dir = switch_dir;
        }
    }

    drawSignals(signals_data, painter);

    drawStations(stations, painter);

    if (train_data == nullptr)
    {
        painter.end();
        return;
    }

    if (follow_player && (follow_player_idx < players_data->current_vehicles.size()))
    {
        // В мультиплеере отслеживаем текущую ПЕ у самого первого подключенного вьювера
        // Следует придумать, как следить за ПЕ выбранного игрока, например себя
        int curr = players_data->current_vehicles[follow_player_idx];
        if ((curr >= 0) && (curr < train_data->vehicles.size()))
        {
            map_shift.setX(- train_data->vehicles[curr].position_y * scale);
            map_shift.setY(- train_data->vehicles[curr].position_x * scale);
        }
    }

    drawTrains(train_data, painter);

    drawTrainNames(train_data, painter);

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrajectory(Trajectory* traj, QPainter& painter,
                               QPointF& cursor_pos, double& distance2)
{
    QPen pen;

    pen.setWidth(1);
    pen.setColor(color_traj_free);

    if (traj->isBusy())
    {
        pen.setWidth(2);
        pen.setColor(color_traj_busy);
    }
    else if (traj->isInRoute())
    {
        pen.setWidth(2);
        pen.setColor(color_traj_route);
    }

    painter.setPen(pen);

    for (const auto& track : traj->getTracks())
    {
        QPoint p0 = coord_transform(track.begin_point);
        QPoint p1 = coord_transform(track.end_point);

        painter.drawLine(p0, p1);

        double track_distance2 = distance2_pos_to_line_segment(cursor_pos, p0, p1);
        if (distance2 > track_distance2)
        {
            distance2 = track_distance2;
        }
    }

    QLabel *traj_label = traj_labels.value(traj->getName());
    if (traj_label != nullptr)
    {
        if (show_traj_names)
        {
            const std::vector<track_t>& tracks = traj->getTracks();
            const track_t& middle_track = tracks[tracks.size() / 2];

            dvec3 mp = (middle_track.begin_point + middle_track.end_point) * 0.5;

            QPoint pt = coord_transform(mp);
            traj_label->move(pt);

            traj_label->show();
        }
        else
        {
            traj_label->hide();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrains(simulator_update_pos_t *train_data, QPainter& painter)
{
    for (size_t i = 0; i < train_data->vehicles.size(); ++i)
    {
        QColor color(64, 128, 0);
        for (auto v_id : players_data->current_vehicles)
        {
            if (i == v_id)
            {
                color = QColor(192, 192, 0);
            }
        }
        for (auto v_id : players_data->controlled_vehicles)
        {
            if (i == v_id)
            {
                color = QColor(192, 64, 64);
            }
        }
        drawVehicle(train_data->vehicles[i], vehicles_half_length->at(i),
                    painter, color);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrainNames(simulator_update_pos_t *train_data, QPainter& painter)
{
    (void) painter;

    if (train_data->vehicles.empty())
    {
        return;
    }

    for (auto tl : train_labels)
    {
        simulator_vehicle_pos_update_t vehicle = train_data->vehicles[tl->first_vehicle_idx];

        dvec3 v_pos;
        v_pos.x = vehicle.position_x;
        v_pos.y = vehicle.position_y;
        v_pos.z = 0.0;
        QPoint text_point = coord_transform(v_pos);

        tl->move(text_point);
        tl->show();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawVehicle(simulator_vehicle_pos_update_t &vehicle, double &vehicle_half_length,
                            QPainter& painter, QColor color)
{
    QPen pen;
    pen.setWidth(5 + std::floor(scale));
    pen.setColor(color);
    pen.setCapStyle(Qt::FlatCap);

    painter.setPen(pen);

    dvec3 fwd;
    fwd.x = vehicle.position_x + vehicle.orth_x * (vehicle_half_length - 0.51);
    fwd.y = vehicle.position_y + vehicle.orth_y * (vehicle_half_length - 0.51);
    fwd.z = 0;

    dvec3 bwd;
    bwd.x = vehicle.position_x - vehicle.orth_x * (vehicle_half_length - 0.51);
    bwd.y = vehicle.position_y - vehicle.orth_y * (vehicle_half_length - 0.51);
    bwd.z = 0;

    QPoint fwd_point = coord_transform(fwd);
    QPoint bwd_point = coord_transform(bwd);

    painter.drawLine(fwd_point, bwd_point);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawConnector(Switch* sw, QPainter& painter,
                              QPointF& cursor_pos, double& distance2, std::int8_t &dir)
{
    if (sw == nullptr)
    {
        return;
    }

    dir_t dir_fwd = FWD;
    Trajectory *fwd_traj = sw->getNextTraj(dir_fwd);
    dir_t dir_bwd = BWD;
    Trajectory *bwd_traj = sw->getNextTraj(dir_bwd);

    switch_coord_t sc = switch_coords.value(sw->getName());
    QPoint center_point = coord_transform(sc.center);

    QLabel *sw_label = switch_labels.value(sw->getName(), nullptr);
    if (sw_label != nullptr)
    {
        if (show_conn_names)
        {
            sw_label->move(center_point);
            sw_label->show();
        }
        else
        {
            sw_label->hide();
        }
    }

    painter.setBrush(color_connector);
    painter.setPen(Qt::NoPen);
    int r = 4 + std::floor(sqrt(scale));
    painter.drawEllipse(center_point, r, r);

    QPen pen;
    int switched_width = 2 + std::floor(sqrt(scale));
    int other_width = 1;
    if (sw->trajectories[SW_FWD_PLUS] && sw->trajectories[SW_FWD_MINUS])
    {
        Trajectory* fwd_other = (fwd_traj == sw->trajectories[SW_FWD_PLUS]) ?
                                    sw->trajectories[SW_FWD_MINUS] : sw->trajectories[SW_FWD_PLUS];

        switch (sw->getStateFwd())
        {
        case IS_BUSY_MINUS:
        case IS_BUSY_PLUS:
        {
            if ((switch_menu.conn == sw) && (switch_menu.switch_dir > 0) &&
                (switch_menu.action != nullptr))
            {
                switch_menu.action->setEnabled(false);
            }

            pen.setColor(color_switch_other);
            pen.setWidth(other_width);
            painter.setPen(pen);
            drawSwitchTraj(fwd_other, true, painter, cursor_pos, distance2, dir);

            pen.setColor(color_switch_busy);
            pen.setWidth(switched_width);
            painter.setPen(pen);
            drawSwitchTraj(fwd_traj, true, painter, cursor_pos, distance2, dir);
            break;
        }
        case IN_ROUTE_MINUS:
        case IN_ROUTE_PLUS:
        {
            if ((switch_menu.conn == sw) && (switch_menu.switch_dir > 0) &&
                (switch_menu.action != nullptr))
            {
                switch_menu.action->setEnabled(false);
            }

            pen.setColor(color_switch_other);
            pen.setWidth(other_width);
            painter.setPen(pen);
            drawSwitchTraj(fwd_other, true, painter, cursor_pos, distance2, dir);

            pen.setColor(color_switch_route);
            pen.setWidth(switched_width);
            painter.setPen(pen);
            drawSwitchTraj(fwd_traj, true, painter, cursor_pos, distance2, dir);
            break;
        }
        default:
        {
            if ((switch_menu.conn == sw) && (switch_menu.switch_dir > 0) &&
                (switch_menu.action != nullptr))
            {
                switch_menu.action->setEnabled(true);
                if ((switch_menu.menu) &&
                    (switch_menu.menu->activeAction() == switch_menu.action))
                {
                    pen.setColor(color_switch_other_selected);
                    pen.setWidth(switched_width);
                }
                else
                {
                    pen.setColor(color_switch_other);
                    pen.setWidth(other_width);
                }
            }
            else
            {
                pen.setColor(color_switch_other);
                pen.setWidth(other_width);
            }

            painter.setPen(pen);
            drawSwitchTraj(fwd_other, true, painter, cursor_pos, distance2, dir);

            pen.setColor(color_switch_free);
            pen.setWidth(switched_width);
            painter.setPen(pen);
            drawSwitchTraj(fwd_traj, true, painter, cursor_pos, distance2, dir);
            break;
        }
        }
    }

    if (sw->trajectories[SW_BWD_PLUS] && sw->trajectories[SW_BWD_MINUS])
    {
        Trajectory* bwd_other = (fwd_traj == sw->trajectories[SW_BWD_PLUS]) ?
                                    sw->trajectories[SW_BWD_MINUS] : sw->trajectories[SW_BWD_PLUS];

        switch (sw->getStateBwd())
        {
        case IS_BUSY_MINUS:
        case IS_BUSY_PLUS:
        {
            if ((switch_menu.conn == sw) && (switch_menu.switch_dir < 0) &&
                (switch_menu.action != nullptr))
            {
                switch_menu.action->setEnabled(false);
            }

            pen.setColor(color_switch_other);
            pen.setWidth(other_width);
            painter.setPen(pen);
            drawSwitchTraj(bwd_other, false, painter, cursor_pos, distance2, dir);

            pen.setColor(color_switch_busy);
            pen.setWidth(switched_width);
            painter.setPen(pen);
            drawSwitchTraj(bwd_traj, false, painter, cursor_pos, distance2, dir);
            break;
        }
        case IN_ROUTE_MINUS:
        case IN_ROUTE_PLUS:
        {
            if ((switch_menu.conn == sw) && (switch_menu.switch_dir < 0) &&
                (switch_menu.action != nullptr))
            {
                switch_menu.action->setEnabled(false);
            }

            pen.setColor(color_switch_other);
            pen.setWidth(other_width);
            painter.setPen(pen);
            drawSwitchTraj(bwd_other, false, painter, cursor_pos, distance2, dir);

            pen.setColor(color_switch_route);
            pen.setWidth(switched_width);
            painter.setPen(pen);
            drawSwitchTraj(bwd_traj, false, painter, cursor_pos, distance2, dir);
            break;
        }
        default:
        {
            if ((switch_menu.conn == sw) && (switch_menu.switch_dir < 0) &&
                (switch_menu.action != nullptr))
            {
                switch_menu.action->setEnabled(true);
                if ((switch_menu.menu) &&
                    (switch_menu.menu->activeAction() == switch_menu.action))
                {
                    pen.setColor(color_switch_other_selected);
                    pen.setWidth(switched_width);
                }
                else
                {
                    pen.setColor(color_switch_other);
                    pen.setWidth(other_width);
                }
            }
            else
            {
                pen.setColor(color_switch_other);
                pen.setWidth(other_width);
            }
            painter.setPen(pen);
            drawSwitchTraj(bwd_other, false, painter, cursor_pos, distance2, dir);

            pen.setColor(color_switch_free);
            pen.setWidth(switched_width);
            painter.setPen(pen);
            drawSwitchTraj(bwd_traj, false, painter, cursor_pos, distance2, dir);
            break;
        }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawSwitchTraj(Trajectory* traj, bool draw_to_fwd, QPainter& painter,
                               QPointF& cursor_pos, double& distance2, std::int8_t& dir)
{
    if (traj == nullptr)
    {
        return;
    }

    double draw_len = traj->getLength() - 1.0;

    if (draw_to_fwd)
    {
        dir_t dir_fwd = FWD;
        if (Switch* next_sw = traj->getNextSwitch(dir_fwd))
        {
            Switch_state_t next_state = (dir_fwd == FWD) ? next_sw->getStateBwd() : next_sw->getStateFwd();
            if (   (next_state != NO_POSSIBLE_DIRECTION)
                && (next_state != ONLY_MINUS)
                && (next_state != ONLY_PLUS))
            {
                draw_len = traj->getLength() * 0.5;
            }
        }
        draw_len = std::min(draw_len, switch_length);

        size_t i = 0;
        dvec3 fwd = traj->getTracks().begin()->begin_point;
        QPoint fwd_point = coord_transform(fwd);
        const track_t* track_next = nullptr;
        while (draw_len > 0.0)
        {
            track_next = &traj->getTracks().at(i);
            fwd += track_next->orth * std::min(draw_len, track_next->len);
            QPoint fwd_point_next = coord_transform(fwd);
            painter.drawLine(fwd_point, fwd_point_next);

            double conn_distance2 = distance2_pos_to_line_segment(cursor_pos, fwd_point, fwd_point_next);
            if (distance2 > conn_distance2)
            {
                distance2 = conn_distance2;
                dir = 1;
            }

            fwd_point = fwd_point_next;
            draw_len = draw_len - track_next->len;
            ++i;
        }
    }
    else
    {
        dir_t dir_bwd = BWD;
        if (Switch* next_sw = traj->getNextSwitch(dir_bwd))
        {
            Switch_state_t next_state = (dir_bwd == BWD) ? next_sw->getStateFwd() : next_sw->getStateBwd();
            if (   (next_state != NO_POSSIBLE_DIRECTION)
                && (next_state != ONLY_MINUS)
                && (next_state != ONLY_PLUS))
            {
                draw_len = traj->getLength() * 0.5;
            }
        }
        draw_len = std::min(draw_len, switch_length);

        size_t i = 1;
        dvec3 bwd = (traj->getTracks().end() - i)->end_point;
        QPoint bwd_point = coord_transform(bwd);
        const track_t* track_next = nullptr;
        while (draw_len > 0.0)
        {
            track_next = &traj->getTracks().at(traj->getTracks().size() - i);
            bwd -= track_next->orth * std::min(draw_len, track_next->len);
            QPoint bwd_point_next = coord_transform(bwd);
            painter.drawLine(bwd_point, bwd_point_next);

            double conn_distance2 = distance2_pos_to_line_segment(cursor_pos, bwd_point, bwd_point_next);
            if (distance2 > conn_distance2)
            {
                distance2 = conn_distance2;
                dir = -1;
            }

            bwd_point = bwd_point_next;
            draw_len = draw_len - track_next->len;
            ++i;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawStations(topology_stations_list_t* stations, QPainter& painter)
{
    if (stations == nullptr)
    {
        return;
    }

    for (auto& station : *stations)
    {
        QFont font("Arial", 14);
        painter.setFont(font);

        dvec3 station_place{station.pos_x, station.pos_y, station.pos_z};
        QPoint place = coord_transform(station_place);

        painter.drawText(place, station.name);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawSignals(signals_data_t *signals_data, QPainter& painter)
{
    if (signals_data == nullptr)
    {
        return;
    }

    for (auto line_signal : signals_data->line_signals)
    {
        if (line_signal)
        {
            if (line_signal->getSignalModel() == "empty_line")
            {
                continue;
            }
            lens_state_t lens = line_signal->getAllLensState();

            std::vector<QColor> lens_colors;
            lens_colors.emplace_back(lens[RED_LENS] ? QColor(255, 0, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[GREEN_LENS] ? QColor(0, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));

            drawSignal(line_signal, painter, lens_colors);
        }
    }

    for (auto enter_signal : signals_data->enter_signals)
    {
        if (enter_signal)
        {
            if (enter_signal->getSignalModel() == "empty_entr")
            {
                continue;
            }
            lens_state_t lens = enter_signal->getAllLensState();

            std::vector<QColor> lens_colors;
            lens_colors.emplace_back(lens[WHITE_LENS] ? QColor(255, 255, 196) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[BOTTOM_YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[RED_LENS] ? QColor(255, 0, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[GREEN_LENS] ? QColor(0, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));

            drawSignal(enter_signal, painter, lens_colors);
        }
    }

    for (auto route_signal : signals_data->route_signals)
    {
        if (route_signal)
        {
            if (route_signal->getSignalModel() == "empty_rout")
            {
                continue;
            }
            lens_state_t lens = route_signal->getAllLensState();

            std::vector<QColor> lens_colors;
            lens_colors.emplace_back(lens[WHITE_LENS] ? QColor(255, 255, 196) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[BOTTOM_YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[RED_LENS] ? QColor(255, 0, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[GREEN_LENS] ? QColor(0, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));

            drawSignal(route_signal, painter, lens_colors);
        }
    }

    for (auto exit_signal : signals_data->exit_signals)
    {
        if (exit_signal)
        {
            if (exit_signal->getSignalModel() == "empty_exit")
            {
                continue;
            }
            lens_state_t lens = exit_signal->getAllLensState();

            std::vector<QColor> lens_colors;
            lens_colors.emplace_back(lens[WHITE_LENS] ? QColor(255, 255, 196) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[RED_LENS] ? QColor(255, 0, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[GREEN_LENS] ? QColor(0, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));

            drawSignal(exit_signal, painter, lens_colors);
        }
    }

    for (auto shunt_signal : signals_data->shunt_signals)
    {
        if (shunt_signal)
        {
            if (shunt_signal->getSignalModel() == "empty_shnt")
            {
                continue;
            }
            lens_state_t lens = shunt_signal->getAllLensState();

            std::vector<QColor> lens_colors;
            lens_colors.emplace_back(lens[BLUE_LENS] ? QColor(0, 96, 255) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[WHITE_LENS] ? QColor(255, 255, 196) : QColor(0, 0, 0));

            drawSignal(shunt_signal, painter, lens_colors);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawSignal(Signal *signal, QPainter& painter, std::vector<QColor> lens_colors)
{
    Switch* sw = signal->getConnector();
    if (sw == nullptr)
    {
        return;
    }

    switch_coord_t sc = switch_coords.value(sw->getName());

    dvec3 bottom_signal_pos = sc.center;
    SignalLabel *signal_label = nullptr;

    if (signal->getDirection() < 0)
    {
        signal_label = signal_labels_bwd.value(sw->getName(), nullptr);
    }
    else
    {
        signal_label = signal_labels_fwd.value(sw->getName(), nullptr);
    }

    bottom_signal_pos += sc.trav * (signal_offset * signal->getDirection());
    double signed_r = signal_radius * signal->getDirection();
    int r = std::round(signal_radius * scale);

    dvec3 label_pos = bottom_signal_pos + sc.orth * ((2 * lens_colors.size() + 3) * signed_r);
    if (signal_label != nullptr)
    {
        QPoint label_p = coord_transform(label_pos);
        label_p.setX(label_p.x() - signal_label->width() / 2);
        label_p.setY(label_p.y() - signal_label->height() / 2);

        signal_label->move(label_p);
        signal_label->show();
    }

    QPen pen;
    pen.setWidth((scale > 5.0) ? 2 : 1);
    painter.setPen(pen);
    for (size_t i = 1; i <= lens_colors.size(); ++i)
    {
        dvec3 lens_pos = bottom_signal_pos + sc.orth * (2 * i * signed_r);
        QPoint lens_point = coord_transform(lens_pos);
        painter.setBrush(lens_colors[i - 1]);
        painter.drawEllipse(lens_point, r, r);
    }

    QPoint bottom_down = coord_transform(bottom_signal_pos);
    QPoint bottom_up = coord_transform(bottom_signal_pos + sc.orth * signed_r);
    QPoint bottom_left = coord_transform(bottom_signal_pos - sc.trav * signed_r);
    QPoint bottom_right = coord_transform(bottom_signal_pos + sc.trav * signed_r);

    pen.setWidth((scale > 2.0) ? 2 : 1);
    painter.setPen(pen);
    painter.drawLine(bottom_down, bottom_up);
    painter.drawLine(bottom_left, bottom_right);
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPoint MapWidget::coord_transform(dvec3 point)
{
    QPoint p;

    // У маршрутов направление вперёд в основном по оси Y,
    // отрисовываем его слева направо - по оси X виджета,
    // т.е. меняем оси местами
    p.setX(this->width() / 2 + map_shift.x() + scale * point.y);
    p.setY(this->height() / 2 + map_shift.y() + scale * point.x);

    return p;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double MapWidget::distance2_pos_to_point(const QPointF& pos, const QPointF& point)
{
    double vec[2] = {double(point.x()) - double(pos.x()), double(point.y()) - double(pos.y())};
    return vec[0] * vec[0] + vec[1] * vec[1];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double MapWidget::distance_pos_to_point(const QPointF& pos, const QPointF& point)
{
    return std::sqrt(distance2_pos_to_point(pos, point));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double MapWidget::distance2_pos_to_line_segment(const QPointF& pos, const QPointF& pointA, const QPointF& pointB)
{
    if (distance2_pos_to_point(pointA, pointB) < 1.0)
    {
        QPointF mean_point = (pointA + pointB) / 2.0;
        return distance2_pos_to_point(pos, mean_point);
    }
    double vec_A_pos[2] = {double(pos.x()) - double(pointA.x()), double(pos.y()) - double(pointA.y())};
    double vec_A_B[2] = {double(pointB.x()) - double(pointA.x()), double(pointB.y()) - double(pointA.y())};
    double t = (vec_A_pos[0] * vec_A_B[0] + vec_A_pos[1] * vec_A_B[1])
               / (vec_A_B[0] * vec_A_B[0] + vec_A_B[1] * vec_A_B[1]);
    t = std::clamp(t, 0.0, 1.0);
    double vec_pos_tAB[2] = {vec_A_B[0] * t - vec_A_pos[0], vec_A_B[1] * t - vec_A_pos[1]};
    return vec_pos_tAB[0] * vec_pos_tAB[0] + vec_pos_tAB[1] * vec_pos_tAB[1];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double MapWidget::distance_pos_to_line_segment(const QPointF& pos, const QPointF& pointA, const QPointF& pointB)
{
    return std::sqrt(distance_pos_to_line_segment(pos, pointA, pointB));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::wheelEvent(QWheelEvent *event)
{
    // Вектор из центра карты к курсору
    QPointF mouse_pos = QPointF(width() / 2.0 - event->position().x(),
                                height() / 2.0 - event->position().y());

    if ((event->angleDelta().y() > 0) && (scale < 16.0))
    {
        scale *= scale_inc_step_coeff;

        QPoint shift_by_mouse_pos = QPointF(mouse_pos * (scale_inc_step_coeff - 1.0)).toPoint();
        map_shift = map_shift * scale_inc_step_coeff + shift_by_mouse_pos;
        prev_map_shift = prev_map_shift * scale_inc_step_coeff + shift_by_mouse_pos;
    }

    if ((event->angleDelta().y() < 0) && (scale > 0.25))
    {
        scale *= scale_dec_step_coeff;

        QPoint shift_by_mouse_pos = QPointF(mouse_pos * (scale_dec_step_coeff - 1.0)).toPoint();
        map_shift = map_shift * scale_dec_step_coeff + shift_by_mouse_pos;
        prev_map_shift = prev_map_shift * scale_dec_step_coeff + shift_by_mouse_pos;
    }

    event->accept();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        map_shift = prev_map_shift + event->pos() - mouse_pos_LBpressed;
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (follow_player)
        {
            follow_player = false;
            prev_map_shift = map_shift;
        }
        mouse_pos_LBpressed = event->pos();
    }

    if (event->button() == Qt::MiddleButton)
    {
        follow_player = true;
    }

    if (event->button() == Qt::RightButton)
    {
        if (nearest_switch)
        {
            emit sigOpenSwitchMenu(nearest_switch, nearest_switch_dir);
        }
        else
        {
            emit sigOpenTrajectoryMenu(nearest_trajectory);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (prev_map_shift != map_shift)
        {
            prev_map_shift = map_shift;
        }
        else
        {
            emit sigSelectNearestTrajectory(nearest_trajectory);
        }
    }
}
