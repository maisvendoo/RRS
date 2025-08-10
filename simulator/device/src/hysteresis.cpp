#include    "hysteresis.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Hysteresis::Hysteresis(double min_value,
                       double max_value,
                       bool init_state)
    : Trigger()
    , min(min_value)
    , max(max_value)
{
    setInitState(init_state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Hysteresis::setRange(double min_value, double max_value)
{
    min = min_value;
    max = max_value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Hysteresis::setValue(double value)
{
    if (value <= min)
        reset();

    if (value >= max)
        set();
}
