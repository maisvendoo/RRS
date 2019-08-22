#include    "trigger.h"

Trigger::Trigger(QObject *parent) : QObject(parent)
    , state(false)
    , old_state(false)
    , onSoundName("on")
    , offSoundName("off")
{

}

Trigger::~Trigger()
{

}

bool Trigger::getState() const
{
    return state;
}

void Trigger::set()
{
    old_state = state;
    state = true;

    if (state != old_state)
        emit soundPlay(onSoundName);
}

void Trigger::reset()
{
    old_state = state;
    state = false;

    if (state != old_state)
        emit soundPlay(offSoundName);
}

void Trigger::setOnSoundName(QString soundName)
{
    onSoundName = soundName;
}

void Trigger::setOffSoundName(QString soundName)
{
    offSoundName = soundName;
}
