#include    "trigger.h"

Trigger::Trigger()
    : state(false)
{

}

bool Trigger::getState() const
{
    return state;
}

void Trigger::set()
{
    state = true;
}

void Trigger::reset()
{
    state = false;
}
