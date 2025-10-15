#include    "trigger-control-timedelay.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TriggerControlTimedelay::TriggerControlTimedelay(double timeout_on, double timeout_off) : TriggerControl()
{
    timer_on = new Timer(timeout_on, false);
    timer_off = new Timer(timeout_off, false);
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
    return (timer_on->isStarted() || (!timer_off->isStarted() && state));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TriggerControlTimedelay::step(double t, double dt)
{
    if (timer_on->step(t, dt))
    {
        timer_on->stop();
        return set();
    }
    if (timer_off->step(t, dt))
    {
        timer_off->stop();
        return reset();
    }

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

        return false;
    }

    // Режим "переключатель" (клавиши включения и отключения совпадают)
    if (is_toogle)
    {
        if ((getKeyState(pressed_keys, key_symbol_on) && isModifier(pressed_keys, key_modifier_on)))
        {
            if (!prev_key)
            {
                prev_key = true; // Запоминаем, что клавиша нажата

                // Команда переключить новым нажатием на клавишу
                state ? resetAfterDelay() : setAfterDelay();
            }
        }
        else
        {
            prev_key = false; // Запоминаем, что клавиша отпущена
        }

        return false;
    }

    // Режим "тумблер" - включение и отключение на разные клавиши
    // Если разные только модификаторы, а управляющая клавиша одинаковая,
    // проверяем новое нажатие на клавишу
    if (key_symbol_off == key_symbol_on)
    {
        if (getKeyState(pressed_keys, key_symbol_on))
        {
            if (!prev_key)
            {
                if (isModifier(pressed_keys, key_modifier_off))
                {
                    prev_key = true; // Запоминаем, что клавиша нажата
                    resetAfterDelay(); // Команда отключить триггер новым нажатием на клавишу
                }
                else
                {
                    if (isModifier(pressed_keys, key_modifier_on))
                    {
                        prev_key = true; // Запоминаем, что клавиша нажата
                        setAfterDelay(); // Команда включить триггер новым нажатием на клавишу
                    }
                }
            }
        }
        else
        {
            prev_key = false; // Запоминаем, что клавиша отпущена
        }
        return false;
    }

    // Режим "тумблер" - включение и отключение на разные клавиши
    if ((getKeyState(pressed_keys, key_symbol_off) && isModifier(pressed_keys, key_modifier_off)))
    {
        resetAfterDelay(); // Команда отключить триггер
    }
    else
    {
        if ((getKeyState(pressed_keys, key_symbol_on) && isModifier(pressed_keys, key_modifier_on)))
        {
            setAfterDelay(); // Команда включить триггер
        }
    }
    return false;
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
