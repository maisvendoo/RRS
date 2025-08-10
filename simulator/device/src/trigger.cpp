#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trigger::setInitState(bool is_state_true)
{
    state = is_state_true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trigger::set()
{
    if (state)
        return;

    state = true;
    sound_change_state.play(); // Изменяем счётчик включений звука
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Trigger::reset()
{
    if (!state)
        return;

    state = false;
    sound_change_state.play(); // Изменяем счётчик включений звука
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
