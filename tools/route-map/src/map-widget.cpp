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
