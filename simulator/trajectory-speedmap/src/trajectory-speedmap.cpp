#include    "trajectory-speedmap.h"
#include    "speedmap.h"

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

    // ТЕСТ: проверяем текущую траекторную координату device
    for (auto device : vehicles_devices)
    {
        device.device->setInputSignal(SpeedMap::INPUT_CURRENT_LIMIT, device.coord);
    }
}

GET_TRAJECTORY_DEVICE(TrajectorySpeedMap)
