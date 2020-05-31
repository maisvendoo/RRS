#include    "electric-brake-regulator.h"

EDBRegulator::EDBRegulator(QObject *parent) : Device(parent)
  , I_max(565)
  , K1(0.1)
  , K2(0.01)
  , dI(0.0)
  , brake_level(0.0)
  , Ia(0.0)
  , If_ref(0.0)
  , Ia_ref(0.0)
  , omega_nom(25.5)
  , omega(0.0)
  , omega_min(0.04)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EDBRegulator::~EDBRegulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double inv(double x)
{
    if (abs(x) <= Physics::ZERO)
        return 1 / Physics::ZERO;
    else
        return 1.0 / x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EDBRegulator::preStep(state_vector_t &Y, double t)
{
    double is_edt = hs_p(abs(omega) - omega_min);

    double If_max = min(I_max, I_max * pow(omega_nom * inv(omega), 2));

    Y[0] = cut(Y[0], 0.0, If_max);

    dI = abs(brake_level) * I_max - Ia;

    If_ref = K1 * dI + Y[0];

    If_ref = cut(If_ref, 0.0, If_max) * is_edt;

    Ia_ref = brake_level * I_max * is_edt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EDBRegulator::ode_system(const state_vector_t &Y,
                                        state_vector_t &dYdt,
                                        double t)
{
    dYdt[0] = K2 * dI;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EDBRegulator::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "I_max", I_max);
    cfg.getDouble(secName, "K1", K1);
    cfg.getDouble(secName, "K2", K2);
    cfg.getDouble(secName, "omega_nom", omega_nom);
    cfg.getDouble(secName, "omega_min", omega_min);
}
