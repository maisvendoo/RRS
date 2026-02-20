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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::setLink(device_coord_t device)
{
    if (device.device != nullptr)
    {
        device.device->link();
        vehicles_devices.push_back(device);
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryDevice *loadTrajectoryDevice(QString lib_path)
{
    TrajectoryDevice *conn_device = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetTrajectoryDevice getTrajectoryDevice = reinterpret_cast<GetTrajectoryDevice>(lib.resolve("getTrajectoryDevice"));

        if (getTrajectoryDevice)
        {
            conn_device = getTrajectoryDevice();
        }
    }

    return conn_device;
}
