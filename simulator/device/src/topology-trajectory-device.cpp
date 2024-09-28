#include    "topology-trajectory-device.h"

#include    <QLibrary>

#include    "filesystem.h"
#include    "Journal.h"

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
void TrajectoryDevice::setFwdConnectorDevice(ConnectorDevice *conn_device)
{
    fwd_conn_device = conn_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::setBwdConnectorDevice(ConnectorDevice *conn_device)
{
    bwd_conn_device = conn_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorDevice *TrajectoryDevice::getFwdConnectorDevice() const
{
    return fwd_conn_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorDevice *TrajectoryDevice::getBwdConnectorDevice() const
{
    return bwd_conn_device;
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
/*
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::setControl(QMap<int, bool> keys,
                        control_signals_t control_signals)
{
    this->keys = QMap<int, bool>(keys);
    this->control_signals = control_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
feedback_signals_t TrajectoryDevice::getFeedback() const
{
    return feedback;
}
*/
/*
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t TrajectoryDevice::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_state_t();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float TrajectoryDevice::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_state_t::createSoundSignal(false);
}
*/

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
