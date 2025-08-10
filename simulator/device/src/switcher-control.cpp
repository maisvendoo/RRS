#include "switcher-control.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitcherControl::SwitcherControl(std::uint16_t key_code, std::uint16_t num_positions) : Switcher(num_positions)
{
    if (key_code)
        setKeyCode(key_code);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherControl::setKeyCode(std::uint16_t key_code)
{
    keyCode = key_code;
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

    bool allow_spring_first = is_spring_first;
    bool allow_spring_last = is_spring_last;

    if (getKeyState(keyCode))
    {
        if (getKeyState(KEY_Alt_L) || getKeyState(KEY_Alt_R))
        {
            // Автовозврат вперёд
            if (allow_spring_first && (state == 0))
                setPosition(state + 1);

            // Автовозврат назад
            if (allow_spring_last && (state == (num_states - 1)))
                setPosition(state - 1);

            return;
        }

        const bool is_ctrl = getKeyState(KEY_Control_L) || getKeyState(KEY_Control_R);
        const bool is_shift = getKeyState(KEY_Shift_L) || getKeyState(KEY_Shift_R);
        if (is_shift && (!is_ctrl))
        {
            if (no_prev_keyCode)
            {
                // Переключение вперёд
                setPosition(state + 1);
                // Запрещаем переключать дальше до отпускания клавиши
                no_prev_keyCode = false;
            }
            // Запрет автовозврата назад
            allow_spring_last = false;
        }
        else
        {
            if (is_ctrl)
            {
                if (no_prev_keyCode)
                {
                    // Переключение назад
                    setPosition(state - 1);
                    // Запрещаем переключать дальше до отпускания клавиши
                    no_prev_keyCode = false;
                }
                // Запрет автовозврата вперёд
                allow_spring_first = false;
            }
        }
    }
    else
    {
        // Разрешаем переключение следующим нажатием клавиши
        no_prev_keyCode = true;
    }

    // Автовозврат вперёд
    if (allow_spring_first && (state == 0))
        setPosition(state + 1);

    // Автовозврат назад
    if (allow_spring_last && (state == (num_states - 1)))
        setPosition(state - 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SwitcherControl::getKeyState(std::uint16_t key) const
{
    if (pressed_keys && key)
    {
        return pressed_keys->count(key);
    }
    return false;
}
