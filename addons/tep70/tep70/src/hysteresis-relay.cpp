#include    "hysteresis-relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HysteresisRelay::HysteresisRelay(double x_min, double x_max, QObject *parent)
  : Device(parent)
  , x(0.0)
  , x_prev(x)
  , x_min(x_min)
  , x_max(x_max)
  , state(0.0)
  , is_active(false)
  , is_locked(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HysteresisRelay::~HysteresisRelay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisRelay::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)

    if (!is_active)
    {
        state = 0;
        return;
    }

    if (is_locked)
    {
        state = 1;
        return;
    }

    if (x < x_min)
        state = 0;

    if (x > x_max)
        state = 1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisRelay::ode_system(const state_vector_t &Y,
                                 state_vector_t &dYdt,
                                 double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisRelay::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
