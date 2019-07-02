#include "gv.h"

GV::GV(QObject* parent) : Device(parent)
  , GVstate(false)
  , VZstate(false)
  , phc(true)
  , sdk(0)
{

}

double GV::getX()
{
    return getY(0);
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

void GV::setGVState(bool state)
{
    this->GVstate = state;
}

void GV::setVZState(bool state)
{
    this->VZstate = state;
}

void GV::setPhc(bool phc)
{
    this->phc = phc;
}

void GV::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    double s01 = static_cast<double>(phc);
    double s02 = static_cast<double>(GVstate);
    double s03 = static_cast<double>(VZstate);


    double on = s03 * s02;
    double off = 1.0 - s02;

    double s1 = Y[0] - 0.95;
    double s2 = on * hs_n(s1);

    double s5 = Fp * pf(Y[0]) - Fk * s02 * s01;

    double s3 = hs_p(Y[0]) * (hs_p(s5) + off);
    double s4 = s2 - s3;
    double s6 = hs_p(s5) * (P0 - Y[1]) *  K1 - Y[1] * K2 * hs_p(sdk);

    sdk = hs_n(Y[1] - P1);
    Uout = Ukr * sdk * hs_p(s1);

    dYdt[0] = Vn * s4;
    dYdt[1] = s6 / Vdk;

}

void GV::load_config(CfgReader& cfg)
{
    cfg.getDouble("GV", "Vn", Vn);
    cfg.getDouble("GV", "Vdk", Vdk);
    cfg.getDouble("GV", "Fk", Fk);
    cfg.getDouble("GV", "Fp", Fp);
    cfg.getDouble("GV", "K1", K1);
    cfg.getDouble("GV", "K2", K2);
    cfg.getDouble("GV", "tVZ", tVZ);
}

void GV::preStep(state_vector_t& Y, double t)
{

}

void GV::stepKeysControl(double t, double dt)
{

}
