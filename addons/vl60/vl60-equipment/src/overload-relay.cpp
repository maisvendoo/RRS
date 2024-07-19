#include    "overload-relay.h"
#include    "filesystem.h"
#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
OverloadRelay::OverloadRelay(QObject *parent) : Trigger(parent)
  , current(0.0)
  , trig_current(800.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
OverloadRelay::~OverloadRelay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OverloadRelay::setCurrent(double value)
{
    current = value;
    if (current > trig_current)
        set();
    else
        reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OverloadRelay::read_config(const QString &filename, const QString &dir_path)
{
    FileSystem &fs = FileSystem::getInstance();
    CfgReader cfg;

    // Custom config from path
    QString cfg_path = dir_path + QDir::separator() + filename + ".xml";
    if (cfg.load(cfg_path))
    {
        cfg.getDouble("Device", "TrigCurren", trig_current);
        return;
    }

    QString cfg_dir = fs.getDevicesDir().c_str();
    cfg_path = cfg_dir + QDir::separator() + filename + ".xml";

    if (cfg.load(cfg_path))
    {
        cfg.getDouble("Device", "TrigCurren", trig_current);
    }
}
