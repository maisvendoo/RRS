#include    "relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Relay::Relay(size_t num_contacts, QObject *parent) : Device(parent)
  , ancor_state_cur(false)
  , ancor_state_prev(false)
  , R(300.0)
  , U(0.0)
  , T(0.1)
  , I_on(1.9)
  , I_off(1.0)
  , I_prev(0.0)
{
    contact.resize(num_contacts);
    std::fill(contact.begin(), contact.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Relay::~Relay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Relay::setInitContactState(size_t number, bool state)
{
    if (number < contact.size())
        contact[number] = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Relay::setVoltage(double U)
{
    this->U = U;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Relay::getContactState(size_t number) const
{
    if (number < contact.size())
        return contact[number];

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Relay::getCurrent() const
{
    return getY(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Relay::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    ancor_state_prev = ancor_state_cur;
    ancor_state_cur = histeresis(Y[0], I_off, I_on);

    if (ancor_state_cur != ancor_state_prev)
        change_contacts_state();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Relay::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(t)

    dYdt[0] = (U / R - Y[0]) / T;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Relay::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    double U_nom = 0;
    cfg.getDouble(secName, "U_nom", U_nom);

    cfg.getDouble(secName, "R", R);
    cfg.getDouble(secName, "T", T);

    double level_off = 0;
    cfg.getDouble(secName, "level_off", level_off);
    I_off = U_nom * level_off / R;

    double level_on = 0;
    cfg.getDouble(secName, "level_on", level_on);
    I_on = U_nom * level_on / R;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Relay::change_contacts_state()
{
    for (size_t i = 0; i < contact.size(); ++i)
    {
        contact[i] = !contact[i];
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Relay::histeresis(double I, double Imin, double Imax)
{
    bool state = false;

    if (I >= Imax)
        state = true;

    if (I < Imin)
        state = false;

    if ( (I >= Imin) && (I < Imax) )
    {
        if (I - I_prev >= 0)
            return false;
        else
            return true;
    }

    I_prev = I;

    return state;
}
