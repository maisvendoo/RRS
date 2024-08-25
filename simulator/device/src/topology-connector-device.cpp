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
void ConnectorDevice::read_config(const QString &filename, const QString &dir_path)
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
