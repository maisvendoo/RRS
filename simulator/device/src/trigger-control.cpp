#include    "trigger-control.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::setKeySymbolOn(std::uint16_t key_symbol)
{
    key_symbol_on = key_symbol;
    checkModeByKeys();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::setKeyModifierOn(std::uint16_t key_modifier)
{
    key_modifier_on = key_modifier;
    checkModeByKeys();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::setKeySymbolOff(std::uint16_t key_symbol)
{
    key_symbol_off = key_symbol;
    checkModeByKeys();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::setKeyModifierOff(std::uint16_t key_modifier)
{
    key_modifier_off = key_modifier;
    checkModeByKeys();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::setControl(std::set<uint16_t>* keys)
{
    pressed_keys = keys;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TriggerControl::step(double t, double dt)
{
    (void) t;
    (void) dt;

    // Режим "кнопка" (не задана клавиша отключения)
    if (is_button)
    {
        if ((getKeyState(pressed_keys, key_symbol_on) && isModifier(pressed_keys, key_modifier_on)))
        {
            return set(); // Нажимаем кнопку вместе с клавишей
        }

        return reset(); // Отпускаем кнопку вместе с клавишей
    }

    // Режим "переключатель" (клавиши включения и отключения совпадают)
    if (is_toogle)
    {
        if ((getKeyState(pressed_keys, key_symbol_on) && isModifier(pressed_keys, key_modifier_on)))
        {
            if (!prev_key)
            {
                // Переключаем новым нажатием на клавишу
                if (state)
                    return reset();
                else
                    return set();
            }
            prev_key = true; // Запоминаем, что клавиша нажата
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
        if (getKeyState(*pressed_keys, key_symbol_on))
        {
            if (!prev_key)
            {
                if (isModifier(*pressed_keys, key_modifier_off))
                {
                    prev_key = true; // Запоминаем, что клавиша нажата
                    return reset(); // Отключаем триггер новым нажатием на клавишу
                }
                if (isModifier(*pressed_keys, key_modifier_on))
                {
                    prev_key = true; // Запоминаем, что клавиша нажата
                    return set(); // Включаем триггер новым нажатием на клавишу
                }
            }
        }
        else
        {
            prev_key = false; // Запоминаем, что клавиша отпущена
        }
        return false;
    }

    // Обычная логика в режиме тумблер
    if ((getKeyState(*pressed_keys, key_symbol_off) && isModifier(*pressed_keys, key_modifier_off)))
    {
        return reset(); // Отключаем триггер
    }
    if ((getKeyState(*pressed_keys, key_symbol_on) && isModifier(*pressed_keys, key_modifier_on)))
    {
        return set(); // Включаем триггер
    }
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::checkModeByKeys()
{
    prev_key = false;
    // Режим "кнопка" (не задана клавиша отключения)
    is_button = ((key_symbol_on != KEY_Undefined) && (key_symbol_off == KEY_Undefined));
    // Режим "переключатель" (клавиши включения и отключения совпадают)
    is_toogle = ((key_symbol_on != KEY_Undefined) && (key_symbol_off == key_symbol_on) && (key_modifier_off == key_modifier_on));
}
