#include    "pkd-15.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeSwitcher::BrakeSwitcher(size_t num_contacts, QObject *parent)
    : Device(parent)
  , tau(1.0)
  , trac_valve_state(false)
  , brake_valve_state(false)
  , p(0.0)
  , p_nom(0.5)
  , omega_nom(0.0)
  , omega(0.0)
  , dir(1)
  , dir_old(dir)
  , mode(1)
{
    setY(0, 1.0);
    contact.resize(num_contacts);
    std::fill(contact.begin(), contact.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeSwitcher::~BrakeSwitcher()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeSwitcher::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    double omg = p * omega_nom / p_nom;
    dir  = static_cast<int>(trac_valve_state) -
            static_cast<double>(brake_valve_state);

    omega = omg * dir;

    Y[0] = cut(Y[0], 0.0, 1.0);

    if ( (Y[0] < 0.05) && (dir == -1) )
    {
        change_contacts_state();
        mode = -1;
    }

    if ( (Y[0] > 0.95) && (dir == 1) )
    {
        change_contacts_state();
        mode = 1;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeSwitcher::ode_system(const state_vector_t &Y,
                               state_vector_t &dYdt,
                               double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    dYdt[0] = omega;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeSwitcher::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "tau", tau);

    if (tau > 1e-10)
    {
        omega_nom = 1.0 / tau;
    }

    cfg.getDouble(secName, "p_nom", p_nom);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeSwitcher::change_contacts_state()
{
    if (dir == dir_old)
        return;

    dir_old = dir;

    for (size_t i = 0; i < contact.size(); ++i)
    {
        contact[i] = !contact[i];
    }
}
