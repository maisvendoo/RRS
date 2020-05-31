#include    "traction-regulator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TractionRegulator::TractionRegulator(QObject *parent) : Device(parent)
  , I_max(565.0)
  , omega_nom(25.5)
  , trac_level(0.0)
  , Ia(0.0)
  , omega(0.0)
  , Ia_ref(0.0)
  , If_ref(0.0)
  , K(50.0)
  , beta_min(0.40)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TractionRegulator::~TractionRegulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double inverse(double x)
{
    if (abs(x) <= Physics::ZERO)
        return 0.0;
    else
        return 1.0 / x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionRegulator::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    // Вычисляем ошибку по скорости КП
    double dw = omega_nom - omega;

    // Рассчитываем ограничение по току якоря
    double Ia_max = trac_level * I_max * hs_p(dw);

    // Рассчитываем ограничение по току на гиперболической ветви характеристики
    double Ig_max = trac_level * I_max * omega_nom * inverse(omega) * (1.0 - hs_p(dw));

    // Рассчитываем заданный ток якоря
    Ia_ref = Ia_max + Ig_max;

    // Вычисляем ошибку по току якоря
    double dI = Ia_ref - Ia;

    // Вычисляем заданный ток возбуждения
    If_ref = I_max - K * dI;

    // Ограничиваем заданный ток возбуждения
    If_ref = cut(If_ref, max(0.0 ,min(beta_min * Ia, beta_min * I_max)), I_max) * hs_p(trac_level);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionRegulator::ode_system(const state_vector_t &,
                                   state_vector_t &,
                                   double)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TractionRegulator::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)

    QString secName = "Device";

    cfg.getDouble(secName, "I_max", I_max);
    cfg.getDouble(secName, "omega_nom", omega_nom);
    cfg.getDouble(secName, "K", K);
    cfg.getDouble(secName, "beta_min", beta_min);
}
