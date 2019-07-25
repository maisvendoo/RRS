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

    DebugMsg = "";

    memory_alloc(1);
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

    preStep(y, t);

    stepControl(t, dt);

    stepDiscrete(t, dt);

    ode_system(y, dydt, t);    

    for (size_t i = 0; i < y.size(); ++i)
    {
        k1[i] = dydt[i];
        y1[i] = y[i] + k1[i] * dt / 2.0;
    }

    ode_system(y1, dydt, t + dt / 2.0);

    for (size_t i = 0; i < y.size(); ++i)
    {
        k2[i] = dydt[i];
        y1[i] = y[i] + k2[i] * dt / 2.0;
    }

    ode_system(y1, dydt, t + dt / 2.0);

    for (size_t i = 0; i < y.size(); ++i)
    {
        k3[i] = dydt[i];
        y1[i] = y[i] + k3[i] * dt;
    }

    ode_system(y1, dydt, t + dt);

    for (size_t i = 0; i < y.size(); ++i)
    {
        k4[i] = dydt[i];
        y[i] = y[i] + (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) * dt / 6.0;
    }

    postStep(y, t + dt);
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

        if (!cfg.getInt(secName, "Order", order))
        {
            order = 1;
        }

        memory_alloc(order);

        load_config(cfg);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::read_custom_config(const QString &path)
{
    CfgReader cfg;

    if (cfg.load(path + ".xml"))
    {
        int order = 0;
        QString secName = "Device";

        if (!cfg.getInt(secName, "Order", order))
        {
            order = 1;
        }

        memory_alloc(order);

        load_config(cfg);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Device::getDebugMsg() const
{
    return DebugMsg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::setControl(QMap<int, bool> keys,
                        control_signals_t control_signals)
{
    this->keys = QMap<int, bool>(keys);
    this->control_signals = control_signals;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
feedback_signals_t Device::getFeedback() const
{
    return feedback;
}

void Device::setCustomConfigDir(const QString &value)
{
    custom_config_dir = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::postStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::memory_alloc(int order)
{
    y.resize(static_cast<size_t>(order));
    dydt.resize(static_cast<size_t>(order));

    y1.resize(static_cast<size_t>(order));

    k1.resize(static_cast<size_t>(order));
    k2.resize(static_cast<size_t>(order));
    k3.resize(static_cast<size_t>(order));
    k4.resize(static_cast<size_t>(order));

    std::fill(y.begin(), y.end(), 0.0);
    std::fill(dydt.begin(), dydt.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::stepExternalControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::stepDiscrete(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Device::getKeyState(int key) const
{
    if (keys.size() == 0)
        return false;

    auto it = keys.find(key);

    if ( it != keys.end() )
        return it.value();

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::stepControl(double t, double dt)
{
    stepKeysControl(t, dt);
    stepExternalControl(t, dt);
}
