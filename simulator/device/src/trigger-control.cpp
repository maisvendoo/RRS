#include    "trigger-control.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TriggerControl::TriggerControl(std::uint16_t key_code) : Trigger()
{
    keyCode = key_code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::setKeyCode(std::uint16_t key_code)
{
    keyCode = key_code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::setControl(std::set<uint16_t> *keys)
{
    pressed_keys = keys;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerControl::step(double t, double dt)
{
    (void) t;
    (void) dt;

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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TriggerControl::getKeyState(std::uint16_t key) const
{
    if (pressed_keys && key)
    {
        return pressed_keys->count(key);
    }
    return false;
}
