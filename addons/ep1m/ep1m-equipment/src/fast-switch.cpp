#include    "fast-switch.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FastSwitch::FastSwitch(QObject *parent) : Relay(4, parent)
  , Uc(0.0)
  , is_power_On(false)
  , is_Hold(false)
  , Id(0.0)
  , Id_max(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FastSwitch::~FastSwitch()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FastSwitch::preStep(state_vector_t &Y, double t)
{
    if ( is_power_On && getContactState(1) )
    {
        power_on_coil.set();
    }

    if ( (!is_Hold) || (qAbs(Id) >= Id_max) )
    {
        power_on_coil.reset();
    }

    setVoltage(Uc * static_cast<double>(power_on_coil.getState()));

    Relay::preStep(Y, t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FastSwitch::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Id_max", Id_max);

    Relay::load_config(cfg);
}
