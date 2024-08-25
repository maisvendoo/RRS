#ifndef     MAP_WIDGET_H
#define     MAP_WIDGET_H

#include    <QWidget>
#include    <QMouseEvent>
#include    <QMap>
#include    <topology-types.h>
#include    <trajectory.h>
#include    <simulator-update-struct.h>
#include    <switch-label.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MapWidget : public QWidget
{
    Q_OBJECT

public:

    MapWidget(QWidget *parent = Q_NULLPTR);

    ~MapWidget();

    traj_list_t *traj_list = Q_NULLPTR;

    conn_list_t *conn_list = Q_NULLPTR;

    tcp_simulator_update_t *train_data = Q_NULLPTR;

    topology_stations_list_t *stations = Q_NULLPTR;

    QMap<QString , SwitchLabel *> switch_labels;

    void resize(int width, int height);

    double getScale() const
    {
        return scale;
    }

    QPoint getMousePos() const
    {
        return mouse_pos;
    }

private:

    /// Масштаб отображения карты
    double scale = 1.0;

    double scale_inc_step_coeff = sqrt(2.0);
    double scale_dec_step_coeff = 1.0 / sqrt(2.0);
/*
    double old_scale = 1.0;

    bool is_stored_old_scale = false;
*/
    /// Смещение координат по горизонтали
    double shift_x = 0;

    /// Смещение координат по вретикали
    double shift_y = 0;

    QPoint map_shift;

    QPoint mouse_pos;


    void paintEvent(QPaintEvent *event);

    void drawTrajectory(Trajectory *traj);

    void drawTrain(tcp_simulator_update_t *train_data);

    void drawVehicle(simulator_vehicle_update_t &vehicle, QColor color);

    void drawConnectors(conn_list_t *conn_list);

    void drawConnector(Connector *conn);

    void drawStations(topology_stations_list_t *stations);

    QPoint coord_transform(dvec3 traj_point);

    void wheelEvent(QWheelEvent *event);

    void mouseMoveEvent(QMouseEvent *event);
};

#endif
