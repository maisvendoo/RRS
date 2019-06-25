#include    "oscillator.h"

Oscillator::Oscillator(QObject *parent) : Device(parent)
  , omega(1.0)
  , beta(1.0)
  , input(0.0)
{

}

Oscillator::~Oscillator()
{

}

double Oscillator::getOutput() const
{
    return getY(0);
}

void Oscillator::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(t)

    dYdt[0] = Y[1];
    dYdt[1] = input - 2 * beta * Y[1] - omega * omega * Y[0];
}

void Oscillator::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Omega", omega);
    cfg.getDouble(secName, "Beta", beta);
}
