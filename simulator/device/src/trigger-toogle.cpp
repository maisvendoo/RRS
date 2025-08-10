#include    "trigger-toogle.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TriggerToogle::TriggerToogle(std::uint16_t key_code) : Trigger()
{
    if (key_code)
        setKeyCode(key_code);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerToogle::setKeyCode(std::uint16_t key_code)
{
    keyCode = key_code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerToogle::setControl(std::set<uint16_t>* keys)
{
    pressed_keys = keys;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TriggerToogle::step(double t, double dt)
{
    (void) t;
    (void) dt;

    if (getKeyState(keyCode))
    {
        if (!is_prev_keyCode)
        {
            if (getKeyState(KEY_Alt_L) || getKeyState(KEY_Alt_R))
            {
                if (getState())
                    reset();
                else
                    set();
            }
            is_prev_keyCode = true;
        }
    }
    else
    {
        is_prev_keyCode = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TriggerToogle::getKeyState(std::uint16_t key) const
{
    if (pressed_keys && key)
    {
        return pressed_keys->count(key);
    }
    return false;
}
