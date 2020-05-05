#include    "vehicle-controller.h"

#include    "connector.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleController::VehicleController(QObject *parent) : QObject(parent)
  , x_prev(0.0)
  , x_cur(0.0)
  , dir(1)
  , traj_coord(0.0)
  , current_traj(Q_NULLPTR)
  , prev_traj(Q_NULLPTR)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleController::~VehicleController()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setRailwayCoord(double x)
{
    x_prev = x_cur;
    x_cur = x;

    double prev_coord = traj_coord;

    traj_coord += dir * (x_cur - x_prev);

    prev_traj = current_traj;

    while (traj_coord > current_traj->getLength())
    {
        Connector *conn = current_traj->getFwdConnector();

        if (conn == Q_NULLPTR)
        {
            traj_coord = prev_coord;
            return;
        }

        traj_coord = traj_coord - current_traj->getLength();

        current_traj = conn->getFwdTraj();

        if (current_traj == Q_NULLPTR)
        {
            current_traj = prev_traj;
            traj_coord = prev_coord;
            break;
        }
    }

    while (traj_coord < 0)
    {
        Connector *conn = current_traj->getBwdConnector();

        if (conn == Q_NULLPTR)
        {
            traj_coord = prev_coord;
            return;
        }

        current_traj = conn->getBwdTraj();

        if (current_traj == Q_NULLPTR)
        {
            current_traj = prev_traj;
            traj_coord = prev_coord;
            break;
        }

        traj_coord = current_traj->getLength() + traj_coord;
    }

    if (current_traj != prev_traj)
    {
        prev_traj->setBusy(false);
        current_traj->setBusy(true);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitRailwayCoord(double x)
{
    x_prev = x_cur = x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleController::setInitCurrentTraj(Trajectory *traj)
{
    current_traj = prev_traj = traj;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vec3d VehicleController::getPosition(vec3d &attitude) const
{
    vec3d pos = current_traj->getPosition(traj_coord, attitude);

    return pos;
}
