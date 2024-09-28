#include    "topology-connector-device.h"

#include    <QLibrary>

#include    "filesystem.h"
#include    "Journal.h"

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
void ConnectorDevice::setConnector(Connector *conn)
{
    connector = conn;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Connector *ConnectorDevice::getConnector() const
{
    return connector;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorDevice::setFwdTrajectoryDevice(TrajectoryDevice *traj_device)
{
    fwd_traj_device = traj_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorDevice::setBwdTrajectoryDevice(TrajectoryDevice *traj_device)
{
    bwd_traj_device = traj_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryDevice *ConnectorDevice::getFwdTrajectoryDevice() const
{
    return fwd_traj_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryDevice *ConnectorDevice::getBwdTrajectoryDevice() const
{
    return bwd_traj_device;
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
/*
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorDevice::setControl(QMap<int, bool> keys,
                        control_signals_t control_signals)
{
    this->keys = QMap<int, bool>(keys);
    this->control_signals = control_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
feedback_signals_t ConnectorDevice::getFeedback() const
{
    return feedback;
}
*/
/*
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t ConnectorDevice::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_state_t();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ConnectorDevice::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_state_t::createSoundSignal(false);
}
*/

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
