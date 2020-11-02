#include "dc-motor-fan.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorFan::DCMotorFan(QObject* parent) : Device(parent)
  , U(0.0)
  , cPhi(0.0)
  , R(0.0)
  , omega_nom(0.0)
  , ks(0.0)
  , J(0.0)
  , soundName("")
  , Unom(0.0)
  , kf(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorFan::~DCMotorFan()
{

}

//------------------------------------------------------------------------------
// Задать напряжение МВ ТЭД
//------------------------------------------------------------------------------
void DCMotorFan::setU(double value)
{
    if (floor(value) > 0 && floor(U) == 0)
    {
        emit soundPlay(soundName);
    }

    if (floor(value) == 0 && floor(U) > 0)
    {
        emit soundStop(soundName);
    }

    U = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::preStep(state_vector_t& Y, double t)
{
    Q_UNUSED(t)

    //emit soundSetPitch(soundName, static_cast<float>(Y[0] / omega_nom));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorFan::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)


    double cphi = kf * sqrt(U);

    double E = Y[0] * cphi;

    double I = (U - E) / R;
    double M = I * cphi;
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
    cfg.getDouble(secName, "ks", ks);
    cfg.getDouble(secName, "J", J);
    cfg.getDouble(secName, "cPhi", cPhi);
    cfg.getDouble(secName, "Unom", Unom);

    kf = cPhi / sqrt(Unom);
}
