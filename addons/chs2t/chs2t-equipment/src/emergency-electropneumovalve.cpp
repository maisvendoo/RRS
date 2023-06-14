#include    "emergency-electropneumovalve.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EmergencyElectroPneumoValve::EmergencyElectroPneumoValve(double min_pressure,
                                     double max_pressure,
                                     QObject *parent)
    : Device(parent)
    , no_emergency(new Hysteresis(min_pressure, max_pressure, false))
    , pFL(0.0)
    , p_add(0.4)
    , p_add_current(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EmergencyElectroPneumoValve::~EmergencyElectroPneumoValve()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EmergencyElectroPneumoValve::setPressureRange(double min_pressure, double max_pressure)
{
    no_emergency->setRange(min_pressure, max_pressure);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EmergencyElectroPneumoValve::setFLpressure(double value)
{
    pFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EmergencyElectroPneumoValve::setBPpressure(double value)
{
    no_emergency->setValue(value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double EmergencyElectroPneumoValve::getAdditionalPressure() const
{
    return p_add_current;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EmergencyElectroPneumoValve::isTractionAllow() const
{
    return no_emergency->getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EmergencyElectroPneumoValve::isEmergency() const
{
    return (!no_emergency->getState());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EmergencyElectroPneumoValve::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    if (no_emergency->getState())
        p_add_current = 0.0;
    else
        p_add_current = min(p_add, pFL);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EmergencyElectroPneumoValve::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EmergencyElectroPneumoValve::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double min_pressure = 0.3;
    double max_pressure = 0.45;
    cfg.getDouble(secName, "p_min", min_pressure);
    cfg.getDouble(secName, "p_max", max_pressure);
    setPressureRange(min_pressure, max_pressure);

    cfg.getDouble(secName, "p_add", p_add);
}
