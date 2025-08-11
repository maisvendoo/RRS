#include    "trigger-control-timedelay.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TriggerControlTimedelay::TriggerControlTimedelay(double timeout_on,
                                                 double timeout_off) : TriggerControl(), QObject()
{
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

    // Режим "кнопка" (не задана клавиша отключения)
    if (is_button)
    {
        if ((getKeyState(pressed_keys, key_symbol_on) && isModifier(pressed_keys, key_modifier_on)))
        {
            setAfterDelay(); // Команда нажать кнопку
        }
        else
        {
            resetAfterDelay(); // Команда отпустить кнопку
        }

        return;
    }

    // Режим "переключатель" (клавиши включения и отключения совпадают)
    if (is_toogle)
    {
        if ((getKeyState(pressed_keys, key_symbol_on) && isModifier(pressed_keys, key_modifier_on)))
        {
            if (!prev_key)
            {
                state ? resetAfterDelay() : setAfterDelay(); // Команда переключить
            }
            prev_key = true; // Запоминаем, что клавиша нажата
        }
        else
        {
            prev_key = false; // Запоминаем, что клавиша отпущена
        }

        return;
    }

    // Режим "тумблер" - включение и отключение на разные клавиши
    if ((getKeyState(*pressed_keys, key_symbol_off) && isModifier(*pressed_keys, key_modifier_off)))
    {
        resetAfterDelay(); // Команда отключить триггер
    }
    else
    {
        if ((getKeyState(*pressed_keys, key_symbol_on) && isModifier(*pressed_keys, key_modifier_on)))
        {
            setAfterDelay(); // Команда включить триггер
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::setAfterDelay()
{
    // Сбрасываем команду отпустить кнопку
    if (timer_off->isStarted())
        timer_off->stop();

    // Команда нажать кнопку
    if (!timer_on->isStarted())
        timer_on->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControlTimedelay::resetAfterDelay()
{
    // Сбрасываем команду нажать кнопку
    if (timer_on->isStarted())
        timer_on->stop();

    // Команда отпустить кнопку
    if (!timer_off->isStarted())
        timer_off->start();
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
