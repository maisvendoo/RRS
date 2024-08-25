#include    "topology-trajectory-device.h"

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
    for (auto device_it = vehicles_devices.begin(); device_it != vehicles_devices.end(); ++device_it)
    {
        if ((*device_it) != nullptr)
        {
            (*device_it)->unlink();
        }
    }
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
void TrajectoryDevice::setLink(Device *device, size_t idx)
{
    if (vehicles_devices.contains(idx))
    {
        if (vehicles_devices[idx] != nullptr)
            vehicles_devices[idx]->unlink();

        vehicles_devices.remove(idx);
    }

    if (device != nullptr)
    {
        device->link();
        vehicles_devices.insert(idx, device);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrajectoryDevice::step(double t, double dt)
{

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
void TrajectoryDevice::read_config(const QString &filename, const QString &dir_path)
{
    FileSystem &fs = FileSystem::getInstance();
    CfgReader cfg;

    // Custom config from path
    if (dir_path != "")
    {
        QString cfg_path = dir_path + QDir::separator() + filename + ".xml";

        if (cfg.load(cfg_path))
        {
            Journal::instance()->info("Loaded file: " + cfg_path);

            load_config(cfg);
            return;
        }
        else
        {
            Journal::instance()->error("File " + filename + ".xml is't found at custom path " + dir_path);
        }
    }

    // Config from default directory
    QString cfg_dir = fs.getDevicesDir().c_str();
    QString cfg_path = cfg_dir + QDir::separator() + filename + ".xml";

    if (cfg.load(cfg_path))
    {
        Journal::instance()->info("Loaded file: " + cfg_path);

        load_config(cfg);
        return;
    }
    Journal::instance()->error("File " + filename + ".xml is't found at default path " + cfg_dir);
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
