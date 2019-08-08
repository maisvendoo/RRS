#include    "current-kind-switch.h"

CurrentKindSwitch::CurrentKindSwitch(QObject *parent) : Device (parent)
  ,state(0)

{

}

CurrentKindSwitch::~CurrentKindSwitch()
{

}

void CurrentKindSwitch::setState(double state)
{

}

void CurrentKindSwitch::setUkrIn(double Ukr_in)
{

}



double CurrentKindSwitch::getUoutDC() const
{
    return Uout_dc;
}

double CurrentKindSwitch::getUotAC() const
{
    return Uout_ac;
}



void CurrentKindSwitch::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)

    Uout_dc = Ukr_in * state;

    Uout_ac = Ukr_in * (1 - state);
}

void CurrentKindSwitch::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{

}

void CurrentKindSwitch::load_config(CfgReader &cfg)
{

}


