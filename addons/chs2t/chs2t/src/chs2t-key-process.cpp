#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::keyProcess()
{
    if (getKeyState(KEY_9))
    {
        if (isShift())
            EDTSwitch.set();
        else
            EDTSwitch.reset();
    }

    if (getKeyState(KEY_V))
    {
        if (isShift())
            epb_switch.set();
        else
            epb_switch.reset();
    }

    state_RB = getKeyState(KEY_M);
    state_RBS = getKeyState(KEY_Z);
}
