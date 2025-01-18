#include    "key-trigger.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeyTrigger::KeyTrigger(int key_code, double timeout_on, double timeout_off, QObject *parent) : Device(parent)
{
    keyCode = key_code;
    timer_on = new Timer(timeout_on, false);
    timer_off = new Timer(timeout_off, false);
    connect(timer_on, &Timer::process, this, &KeyTrigger::slotTimeoutProcessOn);
    connect(timer_off, &Timer::process, this, &KeyTrigger::slotTimeoutProcessOff);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeyTrigger::~KeyTrigger()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::setKeyCode(int key_code)
{
    keyCode = key_code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::setTimeoutOn(double timeout)
{
    timer_on->setTimeout(timeout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::setTimeoutOff(double timeout)
{
    timer_off->setTimeout(timeout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::setInitState(bool state)
{
    state ? trigger.set() : trigger.reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::set()
{
    timer_off->stop();

    if (trigger.getState() || timer_on->isStarted())
        return;

    timer_on->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::reset()
{
    timer_on->stop();

    if (!trigger.getState() || timer_off->isStarted())
        return;

    timer_off->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool KeyTrigger::getState() const
{
    return trigger.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool KeyTrigger::getRefState() const
{
    return (timer_on->isStarted() || (!timer_off->isStarted() && trigger.getState()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t KeyTrigger::getSoundState(size_t idx) const
{
    return trigger.getSoundState(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float KeyTrigger::getSoundSignal(size_t idx) const
{
    return trigger.getSoundSignal(idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::step(double t, double dt)
{
    timer_on->step(t, dt);
    timer_off->step(t, dt);

    if (keyCode > 0)
        stepKeysControl(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::ode_system(const state_vector_t &Y,
                          state_vector_t &dYdt,
                          double t)
{
    (void) Y;
    (void) dYdt;
    (void) t;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::stepKeysControl(double t, double dt)
{
    (void) t;
    (void) dt;

    if (getKeyState(keyCode) && !isAlt())
    {
        isShift() ? set() : reset();
    }
    else
    {
        // Если клавиша была кратковременно нажата и отпущена,
        // обновляем текущее состояние для сброса таймеров срабатывания
        trigger.getState() ? set() : reset();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::slotTimeoutProcessOn()
{
    trigger.set();
    timer_on->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyTrigger::slotTimeoutProcessOff()
{
    trigger.reset();
    timer_off->stop();
}
