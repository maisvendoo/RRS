#include    "trigger.h"

Trigger::Trigger(QObject *parent) : QObject(parent)
    , state(false)
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
    state = true;

    emit soundPlay("Tumbler");
}

void Trigger::reset()
{
    state = false;

    emit soundPlay("Tumbler");
}
