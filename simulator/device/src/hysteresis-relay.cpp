#include    "hysteresis-relay.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HysteresisRelay::HysteresisRelay(double min_value,
                                 double max_value,
                                 bool init_state,
                                 bool is_active,
                                 bool is_locked)
  : Hysteresis(min_value, max_value, init_state)
  , is_active(is_active)
  , is_locked(is_locked)
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
        reset();
        return;
    }

    if (is_locked)
    {
        set();
        return;
    }

    if (value <= min)
        reset();

    if (value >= max)
        set();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool HysteresisRelay::getState() const
{
    return Hysteresis::getState();
}
