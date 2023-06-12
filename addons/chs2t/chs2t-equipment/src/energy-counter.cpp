#include    "energy-counter.h"

EnergyCounter::EnergyCounter(QObject *parent) : Device (parent)
  , P_ks(0.0)
  , P_ptr(0.0)
  , P_trac(0.0)
{

}

EnergyCounter::~EnergyCounter()
{

}

void EnergyCounter::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    dYdt[0] = P_ks / 3600.0 / 1000.0;

    dYdt[1] = P_ptr / 3600.0 / 1000.0;

    dYdt[2] = P_trac / 3600.0 / 1000.0;
}

void EnergyCounter::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    P_trac = P_ks - P_ptr;
}

void EnergyCounter::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
