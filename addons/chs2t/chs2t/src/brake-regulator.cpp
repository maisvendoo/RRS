#include "brake-regulator.h"

BrakeRegulator::BrakeRegulator(QObject* parent) : Device(parent)
{
    u = 0.0;
    k1 = 1250;
    k2 = 5e-3;

}

BrakeRegulator::~BrakeRegulator()
{

}

void BrakeRegulator::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    dYdt[0] = (Bref * k1 * allowEDT - abs(Ia)) * k2;
}

void BrakeRegulator::preStep(state_vector_t& Y, double t)
{
    Y[0] = cut(Y[0], 0.0, 1.0);
    u = Y[0];
}

