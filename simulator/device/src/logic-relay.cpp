#include    "logic-relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LogicRelay::LogicRelay(size_t num_contacts, QObject *parent) : Device(parent)
  , state(false)
  , state_old(state)
{
    contact.resize(num_contacts);
    std::fill(contact.begin(), contact.end(), false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LogicRelay::~LogicRelay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LogicRelay::setInitContactState(size_t number, bool state)
{
    if (number < contact.size())
        contact[number] = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool LogicRelay::getContactState(size_t number) const
{
    if (number < contact.size())
        return contact[number];

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LogicRelay::setState(bool state)
{
    state_old = this->state;
    this->state = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LogicRelay::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    if (state != state_old)
        change_contact_state();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LogicRelay::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LogicRelay::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LogicRelay::change_contact_state()
{
    for (size_t i = 0; i < contact.size(); ++i)
    {
        contact[i] = !contact[i];
    }

    emit changeState();
}
