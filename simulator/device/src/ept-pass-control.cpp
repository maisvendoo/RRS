#include    "ept-pass-control.h"

EPTPassControl::EPTPassControl(QObject *parent) : Device(parent)
  , is_hold(false)
  , is_brake(false)
  , U(0)
  , control_signal(0)
  , lampRelease(0.0f)
  , lampHold(0.0f)
  , lampBrake(0.0f)
{

}

EPTPassControl::~EPTPassControl()
{

}

void EPTPassControl::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    control_signal = 0.0;

    if (is_hold)
        control_signal = -1.0;

    if (is_brake)
        control_signal = 1.0;

    control_signal = control_signal * hs_p(U);

    lampRelease = static_cast<float>(hs_p(U));
    lampHold = static_cast<float>(is_hold * hs_p(U));
    lampBrake = static_cast<float>(is_brake * hs_p(U));
}

void EPTPassControl::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

void EPTPassControl::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
