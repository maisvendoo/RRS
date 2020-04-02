#include    "electric-oil-pump.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectricOilPump::ElectricOilPump(QObject *parent) : Device(parent)
  , U(0.0)
  , Ia(0.0)
  , If(0.0)
  , Ra(0.26)
  , Rf(3)
  , cF(0.56)
  , J(0.1)
  , omega_nom(157.1)
  , kc(0.3)
  , Q_oil(0.0)
  , Q_nom(0.9)
  , is_started(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectricOilPump::~ElectricOilPump()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectricOilPump::getOilFlow() const
{
    return Q_oil;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ElectricOilPump::getCurrent() const
{
    return Ia + If;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectricOilPump::preStep(state_vector_t &Y, double t)
{
    Ia = (U - cF * Y[0] * Physics::sign(If)) / Ra;

    If = U / Rf;

    Q_oil = Q_nom * Y[0] / omega_nom;

    if ( (Y[0] >= 0.1) && !is_started)
    {
        emit soundPlay("Oil_Pump");
        is_started = true;
    }

    if ((Y[0] < 0.1) && is_started)
    {
        emit soundStop("Oil_Pump");
        is_started = false;
    }

    emit soundSetPitch("Oil_Pump", static_cast<float>(Y[0] / omega_nom));
    emit soundSetVolume("Oil_Pump", static_cast<int>(Y[0] * 100 / omega_nom));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectricOilPump::ode_system(const state_vector_t &Y,
                                 state_vector_t &dYdt,
                                 double t)
{
    dYdt[0] = (cF * Ia - kc * Y[0]) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ElectricOilPump::load_config(CfgReader &cfg)
{

}
