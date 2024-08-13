#ifndef     VEHICLE_CONTROLLER_H
#define     VEHICLE_CONTROLLER_H

#include    <QObject>

#include    <topology-types.h>

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

    void setInitCurrentTraj(Trajectory *traj);

    void setTrajCoord(double traj_coord) { this->traj_coord = traj_coord; }

    void setDirection(int dir) { this->dir = dir; }

    //vec3d getPosition(vec3d &attitude) const;

    Trajectory *getCurrentTraj() const { return current_traj; }

    double getTrajCoord() const { return traj_coord; }

protected:

    /// Предыдущее значение дуговой координаты ПЕ
    double  x_prev = 0.0;

    /// Текущее значение дуговой координаты ПЕ
    double  x_cur = 0.0;

    /// Направление движения
    int    dir = 1;

    /// Координата, в пределах текущей траектории
    double traj_coord = 0.0;

    Trajectory *current_traj = Q_NULLPTR;

    Trajectory *prev_traj = Q_NULLPTR;
};

#endif // VEHICLE_CONTROLLER_H
