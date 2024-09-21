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
void MapWidget::paintEvent(QPaintEvent *event)
{
    if (traj_list == Q_NULLPTR)
    {
        return;
    }

    if (conn_list == Q_NULLPTR)
    {
        return;
    }

    if (train_data == Q_NULLPTR)
    {
        return;
    }

    for (auto traj : *traj_list)
    {
        drawTrajectory(traj);
    }

    drawConnectors(conn_list);

    drawTrain(train_data);

    if (stations == Q_NULLPTR)
    {
        return;
    }

    drawStations(stations);

    drawSignals(signals_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrajectory(Trajectory *traj)
{
    QPen pen;
    if (traj->isBusy())
        pen.setColor(QColor(255, 0, 0));

    QPainter painter;
    painter.begin(this);
    painter.setPen(pen);

    for (auto track : traj->getTracks())
    {
        QPoint p0 = coord_transform(track.begin_point);
        QPoint p1 = coord_transform(track.end_point);

        painter.drawLine(p0, p1);
    }

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrain(tcp_simulator_update_t *train_data)
{
    for (size_t i = 0; i < train_data->vehicles.size(); ++i)
    {
        if (i == train_data->controlled_vehicle)
        {
            QColor color(192, 64, 64);
            drawVehicle(train_data->vehicles[i],color);
            continue;
        }
        if (i == train_data->current_vehicle)
        {
            QColor color(192, 192, 0);
            drawVehicle(train_data->vehicles[i],color);
            continue;
        }
        QColor color(64, 128, 0);
        drawVehicle(train_data->vehicles[i],color);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawVehicle(simulator_vehicle_update_t &vehicle, QColor color)
{
    QPen pen;
    pen.setWidth(5 + std::floor(scale));
    pen.setColor(color);
    pen.setCapStyle(Qt::FlatCap);

    QPainter p;
    p.begin(this);
    p.setPen(pen);

    dvec3 fwd;
    fwd.x = vehicle.position_x + vehicle.orth_x * (vehicle.length / 2.0 - 0.45);
    fwd.y = vehicle.position_y + vehicle.orth_y * (vehicle.length / 2.0 - 0.45);
    fwd.z = 0;

    dvec3 bwd;
    bwd.x = vehicle.position_x - vehicle.orth_x * (vehicle.length / 2.0 - 0.45);
    bwd.y = vehicle.position_y - vehicle.orth_y * (vehicle.length / 2.0 - 0.45);
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
    if (conn == Q_NULLPTR)
    {
        return;
    }

    Trajectory *fwd_traj = conn->getFwdTraj();
    Trajectory *bwd_traj = conn->getBwdTraj();

    if ( (fwd_traj == Q_NULLPTR) || (bwd_traj == Q_NULLPTR) )
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

    Switch *sw = dynamic_cast<Switch *>(conn);
    if (sw != Q_NULLPTR)
    {
        if (sw->getStateFwd() != 0)
        {
            QColor color = QColor(0, 0, 128);
            if ((sw->getStateFwd() == 2) || (sw->getStateFwd() == -2))
                color = QColor(96, 96, 96);

            QPen pen;
            pen.setColor(color);
            pen.setWidth(2 + std::floor(sqrt(scale)));
            painter.begin(this);
            painter.setPen(pen);

            double conn_length_fwd = std::min(25.0, fwd_traj->getLength());
            if ((fwd_track.len > conn_length_fwd) || (fwd_traj->getTracks().size() == 1))
            {
                dvec3 fwd = center + fwd_track.orth * conn_length_fwd;
                QPoint fwd_point = coord_transform(fwd);
                painter.drawLine(center_point, fwd_point);
            }
            else
            {
                dvec3 first_fwd = fwd_track.end_point;
                QPoint first_fwd_point = coord_transform(first_fwd);
                painter.drawLine(center_point, first_fwd_point);

                double second_length = conn_length_fwd - fwd_track.len;
                track_t second_fwd_track = *(fwd_traj->getTracks().begin() + 1);
                dvec3 second_fwd = first_fwd + second_fwd_track.orth * second_length;
                QPoint second_fwd_point = coord_transform(second_fwd);
                painter.drawLine(first_fwd_point, second_fwd_point);
            }
            painter.end();
        }

        if (sw->getStateBwd() != 0)
        {
            QColor color = QColor(0, 0, 128);
            if ((sw->getStateBwd() == 2) || (sw->getStateBwd() == -2))
                color = QColor(96, 96, 96);

            QPen pen;
            pen.setColor(color);
            pen.setWidth(2 + std::floor(sqrt(scale)));

            painter.begin(this);
            painter.setPen(pen);

            double conn_length_bwd = std::min(25.0, bwd_traj->getLength());
            if ((bwd_track.len > conn_length_bwd) || (bwd_traj->getTracks().size() == 1))
            {
                dvec3 bwd = center - bwd_track.orth * conn_length_bwd;
                QPoint bwd_point = coord_transform(bwd);
                painter.drawLine(center_point, bwd_point);
            }
            else
            {
                dvec3 first_bwd = bwd_track.begin_point;
                QPoint first_bwd_point = coord_transform(first_bwd);
                painter.drawLine(center_point, first_bwd_point);

                double second_length = conn_length_bwd - bwd_track.len;
                track_t second_bwd_track = *(bwd_traj->getTracks().end() - 2);
                dvec3 second_bwd = first_bwd - second_bwd_track.orth * second_length;
                QPoint second_bwd_point = coord_transform(second_bwd);
                painter.drawLine(first_bwd_point, second_bwd_point);
            }
            painter.end();
        }
    }

    SwitchLabel * sw_label = switch_labels.value(conn->getName(), Q_NULLPTR);

    if (sw_label != Q_NULLPTR)
    {
        sw_label->move(center_point);
        sw_label->show();
    }

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawStations(topology_stations_list_t *stations)
{
    for (auto station : *stations)
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
    if (signals_data == Q_NULLPTR)
    {
        return;
    }

    for (auto line_signal : signals_data->line_signals)
    {
        drawLineSignal(line_signal);
    }

    for (auto enter_signal : signals_data->enter_signals)
    {
        drawEnterSignal(enter_signal);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawEnterSignal(Signal *signal)
{
    if (signal == Q_NULLPTR)
    {
        return;
    }

    Connector *conn = signal->getConnector();

    if (conn == Q_NULLPTR)
    {
        return;
    }

    dvec3 g_signal_pos;
    track_t track;
    Trajectory *traj = conn->getFwdTraj();

    if (traj == Q_NULLPTR)
    {
        traj = conn->getBwdTraj();

        if (traj == Q_NULLPTR)
        {
            return;
        }

        track = traj->getLastTrack();
        g_signal_pos = track.end_point;
    }
    else
    {
        track = traj->getFirstTrack();
        g_signal_pos = track.begin_point;
    }

    double radius = 7.0;
    double right_shift = 30.0;
    g_signal_pos += track.trav * (right_shift * signal->getDirection());
    dvec3 y_signal_pos = g_signal_pos + track.orth * (2 *radius * signal->getDirection());
    dvec3 r_signal_pos = g_signal_pos - track.orth * (2 *radius * signal->getDirection());
    dvec3 by_signal_pos = g_signal_pos - track.orth * (4 *radius * signal->getDirection());
    dvec3 w_signal_pos = g_signal_pos - track.orth * (6 *radius * signal->getDirection());

    QPainter painter;
    painter.begin(this);

    QPoint green_p = coord_transform(g_signal_pos);
    QPoint yellow_p = coord_transform(y_signal_pos);
    QPoint red_p = coord_transform(r_signal_pos);
    QPoint byllow_p = coord_transform(by_signal_pos);
    QPoint white_p = coord_transform(w_signal_pos);

    lens_state_t lens_state = signal->getAllLensState();

    int r = radius * scale;

    QColor g_color(0, 0, 0);
    if (lens_state[GREEN_LENS])
    {
        g_color = QColor(0, 255, 0);
    }
    painter.setBrush(g_color);
    painter.drawEllipse(green_p, r, r);

    QColor y_color(0, 0, 0);
    if (lens_state[YELLOW_LENS])
    {
        y_color = QColor(255, 255, 0);
    }
    painter.setBrush(y_color);
    painter.drawEllipse(yellow_p, r, r);

    QColor r_color(0, 0, 0);
    if (lens_state[RED_LENS])
    {
        r_color = QColor(255, 0, 0);
    }
    painter.setBrush(r_color);
    painter.drawEllipse(red_p, r, r);

    QColor by_color(0, 0, 0);
    if (lens_state[BOTTOM_YELLOW_LENS])
    {
        by_color = QColor(255, 255, 0);
    }
    painter.setBrush(by_color);
    painter.drawEllipse(byllow_p, r, r);

    QColor w_color(0, 0, 0);
    if (lens_state[CALL_LENS])
    {
        w_color = QColor(255, 255, 255);
    }
    painter.setBrush(w_color);
    painter.drawEllipse(white_p, r, r);

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawLineSignal(Signal *signal)
{
    if (signal == Q_NULLPTR)
    {
        return;
    }

    Connector *conn = signal->getConnector();

    if (conn == Q_NULLPTR)
    {
        return;
    }

    dvec3 g_signal_pos;
    track_t track;
    Trajectory *traj = conn->getFwdTraj();

    if (traj == Q_NULLPTR)
    {
        traj = conn->getBwdTraj();

        if (traj == Q_NULLPTR)
        {
            return;
        }

        track = traj->getLastTrack();
        g_signal_pos = track.end_point;
    }
    else
    {
        track = traj->getFirstTrack();
        g_signal_pos = track.begin_point;
    }

    double radius = 7.0;
    double right_shift = 30.0;
    g_signal_pos += track.trav * (right_shift * signal->getDirection());
    dvec3 y_signal_pos = g_signal_pos + track.orth * (2 *radius * signal->getDirection());
    dvec3 r_signal_pos = g_signal_pos - track.orth * (2 *radius * signal->getDirection());

    QPainter painter;
    painter.begin(this);

    QPoint green_p = coord_transform(g_signal_pos);
    QPoint yellow_p = coord_transform(y_signal_pos);
    QPoint red_p = coord_transform(r_signal_pos);

    lens_state_t lens_state = signal->getAllLensState();

    int r = radius * scale;

    QColor g_color(0, 0, 0);
    if (lens_state[GREEN_LENS])
    {
        g_color = QColor(0, 255, 0);
    }
    painter.setBrush(g_color);
    painter.drawEllipse(green_p, r, r);

    QColor y_color(0, 0, 0);
    if (lens_state[YELLOW_LENS])
    {
        y_color = QColor(255, 255, 0);
    }
    painter.setBrush(y_color);
    painter.drawEllipse(yellow_p, r, r);

    QColor r_color(0, 0, 0);
    if (lens_state[RED_LENS])
    {
        r_color = QColor(255, 0, 0);
    }
    painter.setBrush(r_color);
    painter.drawEllipse(red_p, r, r);

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPoint MapWidget::coord_transform(dvec3 point)
{
    QPoint p;

    if (folow_vehicle)
    {
        int curr = train_data->current_vehicle;

        map_shift.setX(- train_data->vehicles[curr].position_y * scale);
        map_shift.setY(- train_data->vehicles[curr].position_x * scale);
    }

    p.setX(this->width() / 2 + map_shift.x() + scale * point.y);
    p.setY(this->height() / 2 + map_shift.y() + scale * point.x);

    return p;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::wheelEvent(QWheelEvent *event)
{
    if ((event->angleDelta().y() > 0) && (scale < 16.0))
    {
        scale *= scale_inc_step_coeff;
        map_shift = map_shift * scale_inc_step_coeff;
        prev_map_shift = prev_map_shift * scale_inc_step_coeff;
    }

    if ((event->angleDelta().y() < 0) && (scale > 0.25))
    {
        scale *= scale_dec_step_coeff;
        map_shift = map_shift * scale_dec_step_coeff;
        prev_map_shift = prev_map_shift * scale_dec_step_coeff;
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
        if (folow_vehicle)
        {
            folow_vehicle = false;
            prev_map_shift = map_shift;
        }
        mouse_pos = event->pos();
    }

    if (event->button() == Qt::MiddleButton)
    {
        folow_vehicle = true;
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
