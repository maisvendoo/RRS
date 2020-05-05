#ifndef     VEHICLE_CONTROLLER_H
#define     VEHICLE_CONTROLLER_H

#include    <QObject>

#include    "topology-types.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehicleController : public QObject
{
    Q_OBJECT

public:

    VehicleController(QObject *parent = Q_NULLPTR);

    virtual ~VehicleController();

    void setRailwayCoord(double x);

    void setInitRailwayCoord(double x);

    void setCurrentTraj(Trajectory *traj);

    void setTrajCoord(double traj_coord) { this->traj_coord = traj_coord; }

    void setDirection(int dir) { this->dir = dir; }

    vec3d getPosition(vec3d &attitude) const;

protected:

    /// Предыдущее значение дуговой координаты ПЕ
    double  x_prev;

    /// Текущее значение дуговой координаты ПЕ
    double  x_cur;

    /// Направление движения
    int    dir;

    /// Координата, в пределах текущей траектории
    double traj_coord;

    Trajectory *current_traj;
};

#endif // VEHICLE_CONTROLLER_H
