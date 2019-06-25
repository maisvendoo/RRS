#include "pantograph.h"


Pantograph::Pantograph(QObject* parent) : Device(parent)
  , tau(0)
  , h(0)
  , state(false)

{

}

double Pantograph::getH()
{
    return h;
}

double Pantograph::getUout()
{
    return Uout;
}

void Pantograph::setUks(double _Uks)
{
    this->Uks = _Uks;
}

void Pantograph::setState(bool state)
{
    this->state = state;
}


void Pantograph::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{

}

void Pantograph::load_config(CfgReader& cfg)
{
    cfg.getDouble("Pantograph", "height", hMax);
    cfg.getDouble("Pantograph", "speed", V);
}

void Pantograph::preStep(state_vector_t& Y, double t)
{

}



void Pantograph::stepKeysControl(double t, double dt)
{
    double s0 = static_cast<double>(state);

    double s2 = s0 * hs_n(h - hMax);

    double s3 = (1.0 - s0) * hs_p(h);

    double s1 = s2 - s3;

    Uout = Uks * hs_p(h - hMax);

    h += s1 * V * dt;
}
