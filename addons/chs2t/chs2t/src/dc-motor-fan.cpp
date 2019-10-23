#include "dc-motor-fan.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorFan::DCMotorFan(QObject* parent) : Device(parent)
  , U(0.0)  
  , R(0.0)
  , omega_nom(0.0)
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
    Q_UNUSED(t)

    emit soundSetPitch(soundName, static_cast<float>(Y[0] / omega_nom));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
<<<<<<< HEAD
    Q_UNUSED(t)

    double E = Y[0] * CPhi * sign(U);
=======
    double E = Y[0] * CPhi * hs_p(U);
>>>>>>> a3a24dc0bc9749b2fabea96fc3b6aec0058bfda9
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

    cfg.getDouble(secName, "R", R);
    cfg.getDouble(secName, "omega_nom", omega_nom);
    cfg.getDouble(secName, "CPhi", CPhi);
    cfg.getDouble(secName, "ks", ks);
    cfg.getDouble(secName, "J", J);
}
