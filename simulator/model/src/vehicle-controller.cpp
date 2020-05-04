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

    traj_coord += dir * (x_cur - x_prev);

    Trajectory *prev_traj = current_traj;

    if (traj_coord < 0)
    {
        current_traj = current_traj->getBwdConnector()->getBwdTraj();

        if (current_traj == Q_NULLPTR)
        {
            current_traj = prev_traj;
            current_traj->setBusy(true);
            traj_coord = 0;
            return;
        }

        traj_coord = current_traj->getLength();

        prev_traj->setBusy(false);
        current_traj->setBusy(true);

        return;
    }

    if (traj_coord > current_traj->getLength())
    {
        current_traj = current_traj->getFwdConnector()->getFwdTraj();

        if (current_traj == Q_NULLPTR)
        {
            current_traj = prev_traj;
            current_traj->setBusy(true);
            traj_coord = current_traj->getLength();
            return;
        }

        traj_coord = 0;

        prev_traj->setBusy(false);
        current_traj->setBusy(true);

        return;
    }
}
