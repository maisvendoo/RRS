#include    "brake-switcher.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeSwitcher::BrakeSwitcher(QObject *parent) : Device(parent)
  , state(1)
  , trac_valve(false)
  , brake_valve(false)
  , shaft_omega_max(2.0)
  , omega(0.0)
  , pos_ref(0.0)
  , K(10.0)
  , eps(0.99)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeSwitcher::~BrakeSwitcher()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeSwitcher::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    pos_ref = static_cast<int>(trac_valve) - static_cast<int>(brake_valve);

    omega = shaft_omega_max * cut(K * (pos_ref - Y[0]), -1.0, 1.0);

    state = static_cast<int>( hs_p(Y[0] - eps) - hs_n(Y[0] + eps) );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeSwitcher::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)

    dYdt[0] = omega;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeSwitcher::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
