#include "pulse-converter.h"

PulseConverter::PulseConverter(QObject* parent) : Device(parent)
  , u(0.0)
  , Uakb(0.0)
  , Ut(0.0)
  , Uf(0.0)
{

}

PulseConverter::~PulseConverter()
{

}

void PulseConverter::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{

}

void PulseConverter::preStep(state_vector_t& Y, double t)
{
    Uf = cut(max(Uakb, cut(abs(Ut), 0.0, 110.0)) * u, 0.0, 107.0);
}