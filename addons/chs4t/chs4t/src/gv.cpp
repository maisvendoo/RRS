#include "gv.h"

GV::GV(QObject* parent) : Device(parent)
  , state(false)
  , phc(true)
  , x(0)
  , x2(0)
  , sdk(0)
{

}

double GV::getX()
{
    return x;
}

double GV::getUout()
{
    return Uout;
}

void GV::setP0(double _P0)
{
    this->P0 = _P0;
}

void GV::setP1(double _P1)
{
    this->P1 = _P1;
}

void GV::setUkr(double Ukr)
{
    this->Ukr = Ukr;
}

void GV::setState(bool state)
{
    this->state = state;
}

void GV::setPhc(bool phc)
{
    this->phc = phc;
}

void GV::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{

}

void GV::load_config(CfgReader& cfg)
{
    cfg.getDouble("GV", "Vn", Vn);
    cfg.getDouble("GV", "Vdk", Vdk);
    cfg.getDouble("GV", "Fk", Fk);
    cfg.getDouble("GV", "Fp", Fp);
    cfg.getDouble("GV", "K1", K1);
    cfg.getDouble("GV", "K2", K2);
}

void GV::preStep(state_vector_t& Y, double t)
{

}

void GV::stepKeysControl(double t, double dt)
{
    double s0 = static_cast<double>(state);
    double s01 = static_cast<double>(phc);

    double s1 = x - 1;
    double s2 = s0 * hs_n(s1);
    double s5 = Fp * hs_p(s1) - Fk * hs_p(s1) * s01;
    double s3 = (1.0 - s0) + hs_p(s5);
    double s4 = s2 - s3;
    double s6 = hs_p(s5) * (P0 - x2) *  K1 - x2 * K2 * hs_p(sdk);
    x += Vn * s4 * dt;
    x2 += s6 / Vdk * dt;
    sdk = hs_n(x2 - P1);
    Uout = Ukr * sdk * hs_p(s1);
}
