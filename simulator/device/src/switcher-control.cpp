#include "switcher-control.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitcherControl::SwitcherControl(std::uint16_t num_positions) : Switcher(num_positions)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::setKeySymbolIncrease(std::uint16_t key_symbol)
{
    key_symbol_inc = key_symbol;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::setKeyModifierIncrease(std::uint16_t key_modifier)
{
    key_modifier_inc = key_modifier;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::setKeySymbolDecrease(std::uint16_t key_symbol)
{
    key_symbol_dec = key_symbol;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::setKeyModifierDecrease(std::uint16_t key_modifier)
{
    key_modifier_dec = key_modifier;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::setControl(std::set<uint16_t>* keys)
{
    pressed_keys = keys;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::setSpringFirst(bool is_spring)
{
    is_spring_first = is_spring;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::setSpringLast(bool is_spring)
{
    is_spring_last = is_spring;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::step(double t, double dt)
{
    (void) t;
    (void) dt;

    // Если нет управляющих сигналов, или если заданы одинаковые клавиши
    // для переключения в следующую/предыдущую позицию, ничего не делаем
    if ((!pressed_keys) || pressed_keys->empty())
    {
        // Автовозврат вперёд
        if (is_spring_first && (state == 0))
            setPosition(state + 1);

        // Автовозврат назад
        if (is_spring_last && (state == (num_states - 1)))
            setPosition(state - 1);

        return;
    }

    bool allow_spring_first = is_spring_first;
    bool allow_spring_last = is_spring_last;

    if ((getKeyState(pressed_keys, key_symbol_dec) && isModifier(pressed_keys, key_modifier_dec)))
    {
        if (!prev_key_dec)
        {
            setPosition(state - 1); // Переключение назад новым нажатием на клавишу
            prev_key_dec = true;    // Запоминаем, что клавиша назад нажата
        }
        // Запрет автовозврата вперёд
        allow_spring_first = false;
    }
    else
    {
        prev_key_dec = false; // Запоминаем, что клавиша назад отпущена

        if ((getKeyState(pressed_keys, key_symbol_inc) && isModifier(pressed_keys, key_modifier_inc)))
        {
            if (!prev_key_inc)
            {
                setPosition(state + 1); // Переключение вперёд новым нажатием на клавишу
                prev_key_inc = true;    // Запоминаем, что клавиша вперёд нажата
            }
            // Запрет автовозврата назад
            allow_spring_last = false;
        }
        else
        {
            prev_key_inc = false; // Запоминаем, что клавиша вперёд отпущена
        }
    }

    // Автовозврат вперёд
    if (allow_spring_first && (state == 0))
        setPosition(state + 1);

    // Автовозврат назад
    if (allow_spring_last && (state == (num_states - 1)))
        setPosition(state - 1);
}
