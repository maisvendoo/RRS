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
#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Device::Device(QObject *parent) : QObject(parent)
  , name("")
  , is_linked(false)
  , sub_step_num(1)
  , solver_type(EULER)
{
    qRegisterMetaType<state_vector_t>();

    custom_cfg_dir = "";
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

    double _dt = dt / static_cast<double>(sub_step_num);
    switch (solver_type)
    {
    case RK4:
    {
        for (size_t i = 0; i < sub_step_num; ++i)
        {
            step_rk4(t + static_cast<double>(i) * _dt, _dt);
        }
        break;
    }
    case EULER2:
    {
        for (size_t i = 0; i < sub_step_num; ++i)
        {
            step_euler2(t + static_cast<double>(i) * _dt, _dt);
        }
        break;
    }
    case EULER:
    default:
    {
        for (size_t i = 0; i < sub_step_num; ++i)
        {
            step_euler(t + static_cast<double>(i) * _dt, _dt);
        }
        break;
    }
    }

    postStep(y, t + dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::link()
{
    is_linked = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::unlink()
{
    is_linked = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Device::isLinked() const
{
    return is_linked;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::setInputSignal(size_t idx, double value)
{
    if (idx < input_signals.size())
        input_signals[idx] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Device::getOutputSignal(size_t idx) const
{
    if (idx < output_signals.size())
        return output_signals[idx];
    else
        return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::setName(QString value)
{
    name = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Device::getName() const
{
    return name;
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
void Device::read_config(const QString &filename, const QString &dir_path)
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

            load_configuration(cfg);
            load_config(cfg);
            return;
        }
        else
        {
            Journal::instance()->error("File " + filename + ".xml is't found at custom path " + dir_path);
        }
    }

    // Custom config from vehicle's subdirectory
    if (custom_cfg_dir != "")
    {
        QString cfg_dir = fs.getVehiclesDir().c_str();
        cfg_dir += fs.separator() + custom_cfg_dir;
        QString cfg_path = cfg_dir + QDir::separator() + filename + ".xml";

        if (cfg.load(cfg_path))
        {
            Journal::instance()->info("Loaded file: " + cfg_path);

            load_configuration(cfg);
            load_config(cfg);
            return;
        }
        else
        {
            Journal::instance()->error("File " + filename + ".xml is't found at custom path " + cfg_dir);
        }
    }

    // Config from default directory
    QString cfg_dir = fs.getDevicesDir().c_str();
    QString cfg_path = cfg_dir + QDir::separator() + filename + ".xml";

    if (cfg.load(cfg_path))
    {
        Journal::instance()->info("Loaded file: " + cfg_path);

        load_configuration(cfg);
        load_config(cfg);
        return;
    }
    Journal::instance()->error("File " + filename + ".xml is't found at default path " + cfg_dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::read_custom_config(const QString &path)
{
    FileSystem &fs = FileSystem::getInstance();
    CfgReader cfg;

    QString cfg_path = QString(fs.getVehiclesDir().c_str()) + QDir::separator() + path + ".xml";

    if (cfg.load(cfg_path))
    {
        Journal::instance()->info("Loaded file: " + cfg_path);
        load_configuration(cfg);
        load_config(cfg);
    }
    else
    {
        Journal::instance()->error("File " + cfg_path + " is't found");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::load_configuration(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);

    int tmp = 0;
    if (!cfg.getInt(secName, "Order", tmp))
    {
        tmp = 1;
    }
    memory_alloc(tmp);

    sub_step_num = 1;
    if (!cfg.getInt(secName, "SubStepNum", tmp))
    {
        sub_step_num = 1;
    }
    else
    {
        if (tmp > 0)
        {
            sub_step_num = tmp;
        }
    }

    solver_type = EULER;
    QString solver_name = "";
    if (!cfg.getString(secName, "Solver", solver_name))
    {
        solver_type = EULER;
    }
    else
    {
        if (solver_name == "rk4")
        {
            solver_type = RK4;
        }
        if (solver_name == "euler2")
        {
            solver_type = EULER2;
        }
        if (solver_name == "euler")
        {
            solver_type = EULER;
        }
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::setCustomConfigDir(const QString &path)
{
    custom_cfg_dir = path;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Device::getCustomConfigDir() const
{
    return custom_cfg_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
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
    //k4.resize(static_cast<size_t>(order));

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
bool Device::isShift() const
{
    return getKeyState(KEY_Shift_L) || getKeyState(KEY_Shift_R);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Device::isControl() const
{
    return getKeyState(KEY_Control_L) || getKeyState(KEY_Control_R);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Device::isAlt() const
{
    return getKeyState(KEY_Alt_L) || getKeyState(KEY_Alt_R);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::stepControl(double t, double dt)
{
    stepKeysControl(t, dt);
    stepExternalControl(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::step_rk4(double t, double dt)
{
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
        //k4[i] = dydt[i];
        //y[i] = y[i] + (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) * dt / 6.0;
        y[i] = y[i] + (k1[i] + 2 * k2[i] + 2 * k3[i] + dydt[i]) * dt / 6.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::step_euler2(double t, double dt)
{
    ode_system(y, dydt, t);

    for (size_t i = 0; i < y.size(); ++i)
    {
        k1[i] = dydt[i];
        y1[i] = y[i] + k1[i] * dt;
    }

    ode_system(y1, dydt, t + dt);

    for (size_t i = 0; i < y.size(); ++i)
    {
        //k2[i] = dydt[i];
        //y[i] = y[i] + (k1[i] + k2[i]) * dt / 2.0;
        y[i] = y[i] + (k1[i] + dydt[i]) * dt / 2.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Device::step_euler(double t, double dt)
{
    ode_system(y, dydt, t);

    for (size_t i = 0; i < y.size(); ++i)
    {
        //k1[i] = dydt[i];
        //y[i] = y[i] + k1[i] * dt;
        y[i] = y[i] + dydt[i] * dt;
    }
}
