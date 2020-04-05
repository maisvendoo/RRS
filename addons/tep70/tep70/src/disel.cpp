#include    "disel.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Disel::Disel(QObject *parent) : Device(parent)
  , V_oil(1.5)
  , Q_emn(0.0)
  , M_sg(0.0)
  , Mc(0.0)
  , ip(3.3)
  , J_shaft(1.0)
  , is_fuel_ignition(false)
  , state_mv6(false)
  , state_vtn(false)
  , n_ref(350.0)
  , Q_max(0.166)
  , Q_fuel(0.0)
  , M_d(0.0)
  , omega_min(19.9)
  , start_time(3.0)
  , timer(new Timer)
  , fuel_pressure(0.0)
  , delta_omega(0.0)
{
    std::fill(K.begin(), K.end(), 0.0);

    K[0] = 15.0;
    K[1] = 0.0774;
    K[2] = K[3] = 1.7e5;
    K[4] = 0.01;

    connect(timer, &Timer::process, this, &Disel::slotFuelIgnition);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Disel::~Disel()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Disel::step(double t, double dt)
{
    timer->step(t, dt);
    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Disel::setQ_emn(double Q_emn)
{
    this->Q_emn = Q_emn;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Disel::setStarterTorque(double M_sg)
{
    this->M_sg = M_sg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Disel::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    if (!is_fuel_ignition)
    {
        if (static_cast<bool>(hs_p(Y[1] - omega_min)))
        {
            timer->start();
        }

        if (Y[1] < 1.0)
            emit soundStop("Disel");
    }
    else
    {
        is_fuel_ignition = state_mv6;
    }

    delta_omega = n_ref * Physics::PI / 30.0 - Y[1];

    Y[2] = cut(Y[2], 0.0, 1.0);

    double s1 = (K[4] * delta_omega + K[5] * Y[2]) * static_cast<double>(state_mv6);

    double u = cut(s1, 0.0, 1.0);

    double Q1 = u * static_cast<double>(state_vtn) * Q_max;

    double Q2 = u * Q_max;

    Q_fuel = Q1 + Q2;

    double M1 = K[2] * Q1;

    double M2 = K[3] * Q2;

    M_d = (M1 + M2) * static_cast<double>(is_fuel_ignition);

    emit soundSetPitch("Disel", static_cast<float>(Y[1] / 36.7));
    emit soundSetVolume("Disel", static_cast<float>(Y[1] * 100.0 / 36.7));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Disel::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(t)

    // Давление масла в системе смазки
    dYdt[0] = (Q_emn + K[1] * Y[1] - K[0] * Y[0]) / V_oil;

    // Частота вращения коленчатого вала
    dYdt[1] = (M_d + ip * M_sg - Mc * Y[1] / 20.0) / J_shaft;

    dYdt[2] = delta_omega;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Disel::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "V_oil", V_oil);
    cfg.getDouble(secName, "Mc", Mc);
    cfg.getDouble(secName, "J_shaft", J_shaft);
    cfg.getDouble(secName, "ip", ip);
    cfg.getDouble(secName, "Qmax", Q_max);
    cfg.getDouble(secName, "omega_min", omega_min);
    cfg.getDouble(secName, "start_time", start_time);

    timer->firstProcess(false);
    timer->setTimeout(start_time);

    for (size_t i = 0; i < K.size(); ++i)
    {
        QString param = QString("K%1").arg(i);
        cfg.getDouble(secName, param, K[i]);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Disel::slotFuelIgnition()
{
    is_fuel_ignition = state_mv6 && static_cast<bool>(hs_p(fuel_pressure - 0.1));
    emit soundPlay("Disel");
    timer->stop();
}
