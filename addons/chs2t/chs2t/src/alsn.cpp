#include    "alsn.h"
#include    "hardware-signals.h"

ALSN::ALSN(QObject *parent) : Device(parent)
  , stateRB1(false)
  , old_stateEPK(false)
  , old_code_alsn(ALSN_WHITE)
{
    safety_relay.reset();
}

ALSN::~ALSN()
{

}

void ALSN::ode_system(const state_vector_t &Y,
                      state_vector_t &dYdt,
                      double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

void ALSN::preStep(state_vector_t &Y, double t)
{
    if (stateRB1)
        safety_relay.set();
}

void ALSN::stepExternalControl(double t, double dt)
{
    if (control_signals.analogSignal[RB1].is_active)
    {
        stateRB1 = static_cast<bool>(control_signals.analogSignal[RB1].cur_value);
    }
}
