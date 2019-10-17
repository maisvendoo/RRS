#include "dc-motor-fan.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorFan::DCMotorFan(QObject* parent) : Device(parent)
  , U(0.0)
  , Un(0.0)
  , In(0.0)
  , Nn(0.0)
  , Pn(0.0)
  , R(0.0)
  , E(0.0)
  , omega_nom(0.0)
  , omega(0.0)
  , CPhi(0.0)
  , ks(0.0)
  , J(0.0)
  , soundName("")
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorFan::~DCMotorFan()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::preStep(state_vector_t& Y, double t)
{
    emit soundSetPitch(soundName, static_cast<float>(Y[0] / 250.0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    double E = Y[0] * CPhi * hs_p(U);
    double I = (U - E) / R;
    double M = I * CPhi;
    double Ms = ks * Y[0] * Y[0] * sign(Y[0]);
    dYdt[0] = (M - Ms) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::load_config(CfgReader& cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Un", Un);
    cfg.getDouble(secName, "In", In);
    cfg.getDouble(secName, "Nn", Nn);
    cfg.getDouble(secName, "Pn", Pn);
    cfg.getDouble(secName, "R", R);
    cfg.getDouble(secName, "omega_nom", omega_nom);
    cfg.getDouble(secName, "CPhi", CPhi);
    cfg.getDouble(secName, "ks", ks);
    cfg.getDouble(secName, "J", J);
}
