#include "hysteresis-polar.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HysteresisPolar::HysteresisPolar(double min_value,
                                 double max_value,
                                 int init_state)
{
    setRange(min_value, max_value);
    setInitState(init_state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisPolar::setInitState(int init_state)
{
    state = init_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisPolar::setRange(double min_value, double max_value)
{
    min = min_value;
    max = max_value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HysteresisPolar::setValue(double value)
{
    if ((value > -min) && (value < min) && (state != 0))
    {
        state = 0;
        sound_change_state.play();
        return;
    }

    if ((value <= -max) && (state != -1))
    {
        state = -1;
        sound_change_state.play();
        return;
    }

    if ((value >= max) && (state != 1))
    {
        state = 1;
        sound_change_state.play();
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int HysteresisPolar::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t HysteresisPolar::getSoundState(size_t idx) const
{
    (void) idx;
    return sound_change_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float HysteresisPolar::getSoundSignal(size_t idx) const
{
    (void) idx;
    return sound_change_state.createSoundSignal();
}
