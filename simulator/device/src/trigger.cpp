#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trigger::setInitState(bool is_state_true)
{
    if (state == is_state_true)
        return false;

    state = is_state_true;
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trigger::set()
{
    if (state)
        return false;

    state = true;
    sound_change_state.play(); // Изменяем счётчик включений звука
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trigger::reset()
{
    if (!state)
        return false;

    state = false;
    sound_change_state.play(); // Изменяем счётчик включений звука
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Trigger::getState() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t Trigger::getSoundState(size_t idx) const
{
    if (idx == CHANGE_SOUND)
        return sound_change_state;
    if (idx == ON_SOUND)
        return sound_state_t(state);
    if (idx == OFF_SOUND)
        return sound_state_t(!state);
    return sound_state_t();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Trigger::getSoundSignal(size_t idx) const
{
    if (idx == CHANGE_SOUND)
        return sound_change_state.createSoundSignal();
    if (idx == ON_SOUND)
        return sound_state_t::createSoundSignal(state);
    if (idx == OFF_SOUND)
        return sound_state_t::createSoundSignal(!state);
    return sound_state_t::createSoundSignal(false);
}
