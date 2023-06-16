#include    "electropneumovalve-release.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroPneumoValveRelease::ElectroPneumoValveRelease(QObject *parent)
    : Device(parent)
    , I(100.0)
    , Ia(0.0)
    , release_valve_state(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroPneumoValveRelease::~ElectroPneumoValveRelease()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroPneumoValveRelease::setEDTcurrent(double value)
{
    Ia = abs(value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ElectroPneumoValveRelease::isPneumoBrakesRelease() const
{
    return release_valve_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroPneumoValveRelease::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    // Разрешение отпуска пневматического тормоза при токе реостата более 100 А
    release_valve_state = (Ia >= I);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroPneumoValveRelease::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectroPneumoValveRelease::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "I", I);
}
