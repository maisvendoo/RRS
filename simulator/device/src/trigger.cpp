#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trigger::Trigger(QObject *parent) : QObject(parent)
    , state(false)
    , was_first_reset(false)
    , sound_change_state(sound_state_t())
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trigger::~Trigger()
{

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
    was_first_reset = true; // Разрешаем звук выключения
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
        return sound_state_t((!state) && was_first_reset);
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
        return sound_state_t::createSoundSignal((!state) && was_first_reset);
    return sound_state_t::createSoundSignal(false);
}
