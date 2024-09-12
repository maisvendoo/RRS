#include    "relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Relay::Relay(size_t num_contacts, QObject *parent) : Device(parent)
  , ancor_state_prev(false)
  , U(0.0)
  , U_nom(110.0)
  , R(157.0)
  , T(0.1)
  , level_off(0.6)
  , level_on(0.7)
  , hysteresis(new Hysteresis(level_off * U_nom / R, level_on * U_nom / R, false))
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
sound_state_t Relay::getSoundState(size_t idx) const
{
    return hysteresis->getSoundState(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Relay::getSoundSignal(size_t idx) const
{
    return hysteresis->getSoundSignal(idx);
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

    hysteresis->setValue(Y[0]);

    if (ancor_state_prev != hysteresis->getState())
        for (auto c_it : contact)
            c_it = !c_it;

    ancor_state_prev = hysteresis->getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Relay::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    Q_UNUSED(t)

    dYdt[0] = (qAbs(U) / R - Y[0]) / T;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Relay::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "U_nom", U_nom);

    double tmp = 0.0;
    cfg.getDouble(secName, "R", tmp);
    if (tmp > Physics::ZERO)
        R = tmp;

    cfg.getDouble(secName, "T", T);

    cfg.getDouble(secName, "level_off", level_off);
    double I_off = U_nom * level_off / R;

    cfg.getDouble(secName, "level_on", level_on);
    double I_on = U_nom * level_on / R;

    hysteresis->setRange(I_off, I_on);
}
