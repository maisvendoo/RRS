#include    "topology-trajectory-device.h"
#include    "topology-connector-device.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryDevice::TrajectoryDevice(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryDevice::~TrajectoryDevice()
{
    clearLinks();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::setTrajectory(Trajectory *traj)
{
    trajectory = traj;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory *TrajectoryDevice::getTrajectory() const
{
    return trajectory;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::setConnectorDevice(ConnectorDevice *conn_device, std::int8_t dir)
{
    if (dir >= 1)
    {
        fwd_conn_device = conn_device;
    }
    else
    {
        bwd_conn_device = conn_device;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorDevice* TrajectoryDevice::getNextConnectorDevice(std::int8_t& dir)
{
    if (dir >= 1)
    {
        if (fwd_conn_device)
        {
            dir = fwd_conn_device->getDeviceOrientation(this);
            return fwd_conn_device;
        }
    }
    if (bwd_conn_device)
    {
        dir = -bwd_conn_device->getDeviceOrientation(this);
        return bwd_conn_device;
    }
    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::clearLinks()
{
    for (auto device : vehicles_devices)
    {
        if (device.device != nullptr)
        {
            device.device->unlink();
        }
    }
    vehicles_devices.clear();
    vehicles_devices_directions.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::setLink(device_coord_t device, std::int8_t direction)
{
    if (device.device != nullptr)
    {
        device.device->link();
        vehicles_devices.push_back(device);
        if (direction > 0)
            vehicles_devices_directions.push_back(1);
        else
            vehicles_devices_directions.push_back(-1);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::step(double t, double dt)
{
    (void) t;
    (void) dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::setInputSignal(size_t idx, double value)
{
    if (idx < input_signals.size())
        input_signals[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TrajectoryDevice::getOutputSignal(size_t idx) const
{
    if (idx < output_signals.size())
        return output_signals[idx];
    else
        return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::setName(QString value)
{
    name = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString TrajectoryDevice::getName() const
{
    return name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::load_config(CfgReader &cfg)
{
    (void) cfg;
}
