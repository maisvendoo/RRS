#include "brake-regulator.h"

BrakeRegulator::BrakeRegulator(QObject* parent) : Device(parent)
{
    u = 0.0;
    k1 = 1.6e-6;
    k2 = 1;

}

BrakeRegulator::~BrakeRegulator()
{

}

void BrakeRegulator::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{

}

void BrakeRegulator::preStep(state_vector_t& Y, double t)
{
    u = cut(k2 * (Bref - k1 * Ia), 0.0, 1.0);
}

