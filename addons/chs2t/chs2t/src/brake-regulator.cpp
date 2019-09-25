#include "brake-regulator.h"

BrakeRegulator::BrakeRegulator(QObject* parent) : Device(parent)
{
    u = 1.0;
}

BrakeRegulator::~BrakeRegulator()
{

}

void BrakeRegulator::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{

}

void BrakeRegulator::preStep(state_vector_t& Y, double t)
{

}

