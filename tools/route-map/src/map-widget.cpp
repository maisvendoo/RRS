#include    <map-widget.h>
#include    <QPainter>
#include    <QWheelEvent>
#include    <connector.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{
    scale = static_cast<double>(this->width()) / 1000.0;
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

    if (!is_stored_old_scale)
    {
        old_scale = scale = static_cast<double>(this->width()) / 2000.0;
        is_stored_old_scale = true;
    }
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


    drawTrain(train_data);

    drawConnectors(conn_list);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawTrajectory(Trajectory *traj)
{
    QPainter painter;
    painter.begin(this);

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
        if (i == 0)
        {
            QColor color(128, 0, 0);
            drawVehicle(train_data->vehicles[i],color);
        }
        else
        {
            QColor color(0, 128, 0);
            drawVehicle(train_data->vehicles[i],color);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::drawVehicle(simulator_vehicle_update_t &vehicle, QColor color)
{
    QPen pen;
    pen.setWidth(6);
    pen.setColor(color);

    QPainter p;
    p.begin(this);
    p.setPen(pen);

    dvec3 fwd;
    fwd.x = vehicle.position_x + vehicle.orth_x * vehicle.length / 2;
    fwd.y = vehicle.position_y + vehicle.orth_y * vehicle.length / 2;
    fwd.z = 0;

    dvec3 bwd;
    bwd.x = vehicle.position_x - vehicle.orth_x * vehicle.length / 2;
    bwd.y = vehicle.position_y - vehicle.orth_y * vehicle.length / 2;
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

    if ( (fwd_traj->getTracks().size() == 0) || (fwd_traj->getTracks().size() == 0) )
    {
        return;
    }

    double conn_length = 5.0;

    track_t fwd_track = fwd_traj->getFirstTrack();
    dvec3 center = fwd_track.begin_point;
    dvec3 fwd = center + fwd_track.orth * conn_length;

    track_t bwd_track = bwd_traj->getLastTrack();
    dvec3 bwd = center - bwd_track.orth * conn_length;

    QPen pen;
    pen.setWidth(4);
    pen.setColor(QColor(0, 0, 128));

    QPainter painter;
    painter.begin(this);
    painter.setPen(pen);

    QPoint center_point = coord_transform(center);
    QPoint fwd_point = coord_transform(fwd);
    QPoint bwd_point = coord_transform(bwd);

    painter.drawLine(center_point, fwd_point);
    painter.drawLine(center_point, bwd_point);

    painter.end();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QPoint MapWidget::coord_transform(dvec3 traj_point)
{
    QPoint p;

    shift_x = this->width() / 2 - train_data->vehicles[0].position_y * scale + map_shift.x();
    shift_y = this->height() / 2 - train_data->vehicles[0].position_x * scale + map_shift.y();

    p.setX(shift_x + scale * traj_point.y);
    p.setY(shift_y + scale * traj_point.x);

    return p;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::wheelEvent(QWheelEvent *event)
{
    scale += event->angleDelta().y() * 0.005;

    if (scale < 0.6)
        scale = 0.6;

    event->accept();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        mouse_pos = event->pos();
    }
}
