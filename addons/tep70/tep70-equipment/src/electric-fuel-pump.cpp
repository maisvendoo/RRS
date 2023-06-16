#include    "electric-fuel-pump.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectricFuelPump::ElectricFuelPump(QObject *parent) : Device(parent)
  , U(0.0)
  , Ia(0.0)
  , If(0.0)
  , Ra(3.74)
  , Rf(40.0)
  , cF(0.5)
  , J(0.01)
  , p_max(0.153)
  , omega_nom(157.1)
  , kc(0.026)
  , level_min(0.01)
  , fuel_level(0.0)
  , fuel_press(0.0)
  , is_started(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectricFuelPump::~ElectricFuelPump()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectricFuelPump::getFuelPressure() const
{
    return fuel_press;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectricFuelPump::getCurrent() const
{
    return Ia + If;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectricFuelPump::setFuelLevel(double fuel_level)
{
    this->fuel_level = fuel_level;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectricFuelPump::setVoltage(double U)
{
    this->U = U;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectricFuelPump::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    fuel_press = p_max * Y[0] * hs_p(fuel_level - level_min) / omega_nom;

    If = U / Rf;

    Ia = (U - cF * Y[0] * Physics::sign(If)) / Ra;

    if ( (Y[0] >= 0.1) && !is_started)
    {
        emit soundPlay("Fuel_Pump");
        is_started = true;
    }

    if ((Y[0] < 0.1) && is_started)
    {
        emit soundStop("Fuel_Pump");
        is_started = false;
    }

    emit soundSetPitch("Fuel_Pump", static_cast<float>(Y[0] / omega_nom));
    emit soundSetVolume("Fuel_Pump", static_cast<int>(Y[0] * 100 / omega_nom));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectricFuelPump::ode_system(const state_vector_t &Y,
                                  state_vector_t &dYdt,
                                  double t)
{
    Q_UNUSED(t)

    dYdt[0] = (cF * Ia - kc * Y[0]) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectricFuelPump::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
