#include    "hysteresis-relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HysteresisRelay::HysteresisRelay(double min_value,
                                 double max_value,
                                 bool init_state,
                                 bool is_active,
                                 bool is_locked,
                                 QObject *parent)
  : Hysteresis(min_value, max_value, init_state, parent)
  , is_active(is_active)
  , is_locked(is_locked)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HysteresisRelay::~HysteresisRelay()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisRelay::setActive(bool is_active)
{
    this->is_active = is_active;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisRelay::setLocked(bool is_locked)
{
    this->is_locked = is_locked;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisRelay::setValue(double value)
{
    if (!is_active)
    {
        state.reset();
        return;
    }

    if (is_locked)
    {
        state.set();
        return;
    }

    if (value <= min)
        state.reset();

    if (value >= max)
        state.set();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool HysteresisRelay::getState() const
{
    return Hysteresis::getState();
}
