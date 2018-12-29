//------------------------------------------------------------------------------
//
//      Abstract class for train devices
//      (c) maisvendoo, 27/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Abstract class for train devices
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 27/12/2018
 */

#include    "device.h"
#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Device::Device(QObject *parent) : QObject(parent)
{
    FileSystem &fs = FileSystem::getInstance();
    cfg_dir = fs.getDevicesDir();
    modules_dir = fs.getModulesDir();

    qRegisterMetaType<state_vector_t>();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Device::~Device()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::step(double t, double dt)
{
    emit DebugPrint(t, y);

    ode_system(y, dydt, t);

    for (size_t i = 0; i < y.size(); ++i)
        y[i] += dydt[i] * dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::setY(size_t i, double value)
{
    if (i < y.size())
        y[i] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Device::getY(size_t i) const
{
    if (i < y.size())
        return y[i];

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::read_config(const QString &path)
{
    CfgReader cfg;
    QString cfg_path = QString(cfg_dir.c_str()) + QDir::separator() + path + ".xml";

    if (cfg.load(cfg_path))
    {
        int order = 0;
        QString secName = "Device";

        if (cfg.getInt(secName, "Order", order))
        {
            y.resize(static_cast<size_t>(order));
            dydt.resize(static_cast<size_t>(order));

            std::fill(y.begin(), y.end(), 0.0);
            std::fill(dydt.begin(), dydt.end(), 0.0);
        }

        load_config(cfg);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
