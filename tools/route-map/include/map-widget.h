#ifndef     MAP_WIDGET_H
#define     MAP_WIDGET_H

#include    <QWidget>
#include    <topology-types.h>
#include    <trajectory.h>
#include    <simulator-update-struct.h>

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

    void resize(int width, int height);

private:

    /// Масштаб отображения карты
    double scale = 1.0;

    /// Смещение координат по горизонтали
    double shift_x = 0;

    /// Смещение координат по вретикали
    double shift_y = 0;

    void paintEvent(QPaintEvent *event);

    void drawTrajectory(Trajectory *traj);

    QPoint coord_transform(dvec3 traj_point);
};

#endif
