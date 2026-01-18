#include    <map-widget.h>
#include    <QPainter>
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
void MapWidget::paintEvent(QPaintEvent *event)
{
    (void)event;

    QPainter painter(this);
    painter.fillRect(rect(), QColor(150, 150, 150));
    QWidget::paintEvent(event);

    if (traj_list == nullptr)
    {
        return;
    }

    if (conn_list == nullptr)
    {
        return;
    }

    for (auto traj : *traj_list)
    {
        drawTrajectory(traj);
    }

    if (train_data == nullptr)
    {
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

    drawTrain(train_data);

    drawTrainNames(train_data);

    drawConnectors(conn_list);

    drawSignals(signals_data);

    if (stations == nullptr)
    {
        return;
    }

    drawStations(stations);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrajectory(Trajectory *traj)
{
    QPen pen;

    pen.setWidth(1);

    if (traj->isBusy())
    {
        pen.setWidth(2);
        pen.setColor(QColor(255, 0, 0));
    }
    else if (traj->isInRoute())
    {
        pen.setWidth(2);
        pen.setColor(QColor(255, 255, 0));
    }

    QPainter painter;
    painter.begin(this);
    painter.setPen(pen);

    for (auto track : traj->getTracks())
    {
        QPoint p0 = coord_transform(track.begin_point);
        QPoint p1 = coord_transform(track.end_point);

        painter.drawLine(p0, p1);
    }


    std::vector<track_t> tracs = traj->getTracks();
    track_t middle_track = tracs[tracs.size() / 2];

    dvec3 mp = (middle_track.begin_point + middle_track.end_point) * 0.5;

    QPoint pt = coord_transform(mp);

    QLabel *traj_label = traj_labels.value(traj->getName());

    if (traj_label != nullptr)
    {
        traj_label->move(pt);

        if (show_traj_names)
            traj_label->show();
        else
            traj_label->hide();
    }

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrain(simulator_update_pos_t *train_data)
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
        drawVehicle(train_data->vehicles[i], vehicles_half_length->at(i), color);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrainNames(simulator_update_pos_t *train_data)
{
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
void MapWidget::drawVehicle(simulator_vehicle_pos_update_t &vehicle, double &vehicle_half_length, QColor color)
{
    QPen pen;
    pen.setWidth(5 + std::floor(scale));
    pen.setColor(color);
    pen.setCapStyle(Qt::FlatCap);

    QPainter p;
    p.begin(this);
    p.setPen(pen);

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

    p.drawLine(fwd_point, bwd_point);

    p.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawConnectors(conn_list_t *conn_list)
{
    for (auto conn : *conn_list)
    {
        drawConnector(conn);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawConnector(Connector *conn)
{
    if (conn == nullptr)
    {
        return;
    }

    Trajectory *fwd_traj = conn->getFwdTraj();
    Trajectory *bwd_traj = conn->getBwdTraj();

    if ( (fwd_traj == nullptr) || (bwd_traj == nullptr) )
    {
        return;
    }

    if ( (fwd_traj->getTracks().size() == 0) || (bwd_traj->getTracks().size() == 0) )
    {
        return;
    }

    track_t fwd_track = fwd_traj->getFirstTrack();
    track_t bwd_track = bwd_traj->getLastTrack();
    dvec3 center = fwd_track.begin_point;
    QPoint center_point = coord_transform(center);

    QColor color = QColor(96, 96, 96);
    int r = 4 + std::floor(sqrt(scale));

    QPainter painter;
    painter.begin(this);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center_point, r, r);
    painter.end();

    SwitchLabel *sw_label = switch_labels.value(conn->getName(), nullptr);

    if (sw_label != nullptr)
    {
        sw_label->move(center_point);
        sw_label->show();
    }

    Switch *sw = dynamic_cast<Switch *>(conn);
    if (sw != nullptr)
    {
        if (sw->getStateFwd() != Switch::ONE_POSSIBLE_DIRECTION)
        {
            QColor color = QColor(0, 0, 128);
            if ((sw->getStateFwd() == Switch::IS_BUSY_PLUS) || (sw->getStateFwd() == Switch::IS_BUSY_MINUS))
            {
                color = QColor(96, 96, 96);

                if ((sw_label != nullptr) && (sw_label->menu != nullptr) &&
                    (sw_label->action_switch_fwd != nullptr))
                {
                    sw_label->action_switch_fwd->setEnabled(false);
                }
            }
            else if ((sw->getStateFwd() == Switch::IN_ROUTE_PLUS) || (sw->getStateFwd() == Switch::IN_ROUTE_MINUS))
            {
                color = QColor(255, 192, 96);

                if ((sw_label != nullptr) && (sw_label->menu != nullptr) &&
                    (sw_label->action_switch_fwd != nullptr))
                {
                    sw_label->action_switch_fwd->setEnabled(false);
                }
            }
            else
            {
                if ((sw_label != nullptr) && (sw_label->menu != nullptr) &&
                    (sw_label->action_switch_fwd != nullptr))
                {
                    sw_label->action_switch_fwd->setEnabled(true);

                    if (sw_label->action_switch_fwd == sw_label->menu->activeAction())
                    {
                        color = QColor(0, 128, 255);
                    }
                }
            }

            QPen pen;
            pen.setColor(color);
            pen.setWidth(2 + std::floor(sqrt(scale)));
            painter.begin(this);
            painter.setPen(pen);

            double conn_length_fwd = std::min(switch_length, std::max(fwd_traj->getLength() - 1.0, 15.0));
            dvec3 fwd = center;
            QPoint fwd_point = center_point;
            track_t track_next = fwd_track;
            size_t i = 0;
            while (conn_length_fwd > 0.0)
            {
                track_next = *(fwd_traj->getTracks().begin() + i);
                fwd += track_next.orth * std::min(conn_length_fwd, track_next.len);
                QPoint fwd_point_next = coord_transform(fwd);
                painter.drawLine(fwd_point, fwd_point_next);

                fwd_point = fwd_point_next;
                conn_length_fwd = conn_length_fwd - track_next.len;
                ++i;
            }

            painter.end();
        }

        if (sw->getStateBwd() != Switch::ONE_POSSIBLE_DIRECTION)
        {
            QColor color = QColor(0, 0, 128);
            if ((sw->getStateBwd() == Switch::IS_BUSY_PLUS) || (sw->getStateBwd() == Switch::IS_BUSY_MINUS))
            {
                color = QColor(96, 96, 96);

                if ((sw_label != nullptr) && (sw_label->menu != nullptr) &&
                    (sw_label->action_switch_bwd != nullptr))
                {
                    sw_label->action_switch_bwd->setEnabled(false);
                }
            }
            else if ((sw->getStateBwd() == Switch::IN_ROUTE_PLUS) || (sw->getStateBwd() == Switch::IN_ROUTE_MINUS))
            {
                color = QColor(255, 192, 96);

                if ((sw_label != nullptr) && (sw_label->menu != nullptr) &&
                    (sw_label->action_switch_bwd != nullptr))
                {
                    sw_label->action_switch_bwd->setEnabled(false);
                }
            }
            else
            {
                if ((sw_label != nullptr) && (sw_label->menu != nullptr) &&
                    (sw_label->action_switch_bwd != nullptr))
                {
                    sw_label->action_switch_bwd->setEnabled(true);

                    if (sw_label->action_switch_bwd == sw_label->menu->activeAction())
                    {
                        color = QColor(0, 128, 255);
                    }
                }
            }

            QPen pen;
            pen.setColor(color);
            pen.setWidth(2 + std::floor(sqrt(scale)));

            painter.begin(this);
            painter.setPen(pen);

            double conn_length_bwd = std::min(switch_length, std::max(bwd_traj->getLength() - 1.0, 15.0));
            dvec3 bwd = center;
            QPoint bwd_point = center_point;
            track_t track_next = bwd_track;
            size_t i = 1;
            while (conn_length_bwd > 0.0)
            {
                track_next = *(bwd_traj->getTracks().end() - i);
                bwd -= track_next.orth * std::min(conn_length_bwd, track_next.len);
                QPoint bwd_point_next = coord_transform(bwd);
                painter.drawLine(bwd_point, bwd_point_next);

                bwd_point = bwd_point_next;
                conn_length_bwd = conn_length_bwd - track_next.len;
                ++i;
            }

            painter.end();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawStations(topology_stations_list_t* stations)
{
    for (auto& station : *stations)
    {
        QPainter painter;
        painter.begin(this);
        QFont font("Arial", 14);
        painter.setFont(font);

        dvec3 station_place{station.pos_x, station.pos_y, station.pos_z};
        QPoint place = coord_transform(station_place);

        painter.drawText(place, station.name);

        painter.end();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawSignals(signals_data_t *signals_data)
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
            lens_colors.emplace_back(lens[GREEN_LENS] ? QColor(0, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[RED_LENS] ? QColor(255, 0, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));

            drawSignal(line_signal, lens_colors);
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
            lens_colors.emplace_back(lens[GREEN_LENS] ? QColor(0, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[RED_LENS] ? QColor(255, 0, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));

            drawSignal(enter_signal, lens_colors);
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
            lens_colors.emplace_back(lens[GREEN_LENS] ? QColor(0, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[RED_LENS] ? QColor(255, 0, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));

            drawSignal(route_signal, lens_colors);
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
            lens_colors.emplace_back(lens[GREEN_LENS] ? QColor(0, 255, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[RED_LENS] ? QColor(255, 0, 0) : QColor(0, 0, 0));
            lens_colors.emplace_back(lens[YELLOW_LENS] ? QColor(255, 255, 0) : QColor(0, 0, 0));

            drawSignal(exit_signal, lens_colors);
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

            drawSignal(shunt_signal, lens_colors);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawSignal(Signal *signal, std::vector<QColor> lens_colors)
{
    Connector *conn = signal->getConnector();
    if ((conn == nullptr) || (signal->getDirection() == 0))
    {
        return;
    }

    Trajectory *traj = (signal->getDirection() < 0) ? conn->getFwdTraj() : conn->getBwdTraj();
    if (traj == nullptr)
    {
        return;
    }

    dvec3 bottom_signal_pos;
    track_t track;
    SignalLabel *signal_label = nullptr;
    if (signal->getDirection() < 0)
    {
        track = traj->getFirstTrack();
        bottom_signal_pos = track.begin_point;
        signal_label = signal_labels_bwd.value(conn->getName(), nullptr);
    }
    else
    {
        track = traj->getLastTrack();
        bottom_signal_pos = track.end_point;
        signal_label = signal_labels_fwd.value(conn->getName(), nullptr);
    }

    bottom_signal_pos += track.trav * (signal_offset * signal->getDirection());
    double signed_r = signal_radius * signal->getDirection();
    int r = signal_radius * scale;

    dvec3 label_pos = bottom_signal_pos + track.orth * ((2 * lens_colors.size() + 3) * signed_r);
    if (signal_label != nullptr)
    {
        QPoint label_p = coord_transform(label_pos);
        label_p.setX(label_p.x() - signal_label->width() / 2);
        label_p.setY(label_p.y() - signal_label->height() / 2);

        signal_label->move(label_p);
        signal_label->show();
    }

    QPainter painter;
    painter.begin(this);

    for (size_t i = 1; i <= lens_colors.size(); ++i)
    {
        dvec3 lens_pos = bottom_signal_pos + track.orth * (2 * i * signed_r);
        QPoint lens_point = coord_transform(lens_pos);
        painter.setBrush(lens_colors[i - 1]);
        painter.drawEllipse(lens_point, r, r);
    }

    QPoint bottom_down = coord_transform(bottom_signal_pos);
    QPoint bottom_up = coord_transform(bottom_signal_pos + track.orth * signed_r);
    QPoint bottom_left = coord_transform(bottom_signal_pos - track.trav * signed_r);
    QPoint bottom_right = coord_transform(bottom_signal_pos + track.trav * signed_r);

    QPen pen;
    pen.setWidth((scale > 1.0) ? 2 : 1);
    painter.setPen(pen);
    painter.drawLine(bottom_down, bottom_up);
    painter.drawLine(bottom_left, bottom_right);

    painter.end();
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
        map_shift = prev_map_shift + event->pos() - mouse_pos;
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
        mouse_pos = event->pos();
    }

    if (event->button() == Qt::MiddleButton)
    {
        follow_player = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        prev_map_shift = map_shift;
    }
}
