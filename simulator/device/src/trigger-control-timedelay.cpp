#include    "trigger-control-timedelay.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TriggerControlTimedelay::TriggerControlTimedelay(std::uint16_t key_code,
                                                 double timeout_on,
                                                 double timeout_off) : TriggerControl(), QObject()
{
    keyCode = key_code;
    timer_on = new Timer(timeout_on, false);
    timer_off = new Timer(timeout_off, false);
    connect(timer_on, &Timer::process, this, &TriggerControlTimedelay::slotTimeoutProcessOn);
    connect(timer_off, &Timer::process, this, &TriggerControlTimedelay::slotTimeoutProcessOff);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TriggerControlTimedelay::~TriggerControlTimedelay()
{
    delete timer_on;
    delete timer_off;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::setTimeoutOn(double timeout)
{
    timer_on->setTimeout(timeout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::setTimeoutOff(double timeout)
{
    timer_off->setTimeout(timeout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::set()
{
    timer_off->stop();

    if (getState() || timer_on->isStarted())
        return;

    timer_on->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::reset()
{
    timer_on->stop();

    if (!getState() || timer_off->isStarted())
        return;

    timer_off->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TriggerControlTimedelay::getRefState() const
{
    return (timer_on->isStarted() || (!timer_off->isStarted() && getState()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::step(double t, double dt)
{
    timer_on->step(t, dt);
    timer_off->step(t, dt);

    if (getKeyState(keyCode))
    {
        if (getKeyState(KEY_Alt_L) || getKeyState(KEY_Alt_R))
            return;

        bool is_ctrl = getKeyState(KEY_Control_L) || getKeyState(KEY_Control_R);
        bool is_shift = getKeyState(KEY_Shift_L) || getKeyState(KEY_Shift_R);
        if (is_shift && (!is_ctrl))
        {
            set();
        }
        else
        {
            if (is_ctrl)
            {
                reset();
            }
        }
    }
    else
    {
        // Если клавиша была кратковременно нажата и отпущена,
        // обновляем текущее состояние для сброса таймеров срабатывания
        getState() ? set() : reset();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::slotTimeoutProcessOn()
{
    set();
    timer_on->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::slotTimeoutProcessOff()
{
    reset();
    timer_off->stop();
}
