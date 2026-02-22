#include    "topology-connector-device.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorDevice::ConnectorDevice(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorDevice::~ConnectorDevice()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorDevice::setConnector(Switch* conn)
{
    connector = conn;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch* ConnectorDevice::getConnector() const
{
    return connector;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorDevice::setTrajectoryDevice(TrajectoryDevice* traj_device, std::int8_t dir, std::int8_t orient)
{
    if (dir >= 1)
    {
        fwd_traj_device = traj_device;
        (orient >= 1) ? (fwd_dir = 1) : (fwd_dir = -1);
    }
    else
    {
        bwd_traj_device = traj_device;
        (orient >= 1) ? (bwd_dir = 1) : (bwd_dir = -1);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryDevice* ConnectorDevice::getNextTrajectoryDevice(std::int8_t& dir) const
{
    if (dir >= 1)
    {
        dir = fwd_dir;
        return fwd_traj_device;
    }
    dir = -bwd_dir;
    return bwd_traj_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::int8_t ConnectorDevice::getDeviceOrientation(TrajectoryDevice* traj_device) const
{
    if (traj_device == fwd_traj_device)
    {
        return fwd_dir;
    }
    if (traj_device == bwd_traj_device)
    {
        return bwd_dir;
    }
    return 1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorDevice::step(double t, double dt)
{
    (void) t;
    (void) dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorDevice::setName(QString value)
{
    name = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ConnectorDevice::getName() const
{
    return name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorDevice::load_config(CfgReader &cfg)
{
    (void) cfg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorDevice *loadConnectorDevice(QString lib_path)
{
    ConnectorDevice *conn_device = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetConnectorDevice getConnectorDevice = reinterpret_cast<GetConnectorDevice>(lib.resolve("getConnectorDevice"));

        if (getConnectorDevice)
        {
            conn_device = getConnectorDevice();
        }
    }

    return conn_device;
}
