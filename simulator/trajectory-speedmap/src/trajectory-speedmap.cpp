#include    "trajectory-speedmap.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectorySpeedMap::TrajectorySpeedMap(QObject *parent) : TrajectoryDevice(parent)
{
    name = QString("speedmap");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectorySpeedMap::~TrajectorySpeedMap()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectorySpeedMap::step(double t, double dt)
{
    (void) t;
    (void) dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectorySpeedMap::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);
}

GET_TRAJECTORY_DEVICE(TrajectorySpeedMap)
