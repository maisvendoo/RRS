#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::keyProcess()
{
    if (getKeyState(KEY_R))
    {
        if (isShift())
            EDTSwitch.set();
        else
            EDTSwitch.reset();
    }

    if (getKeyState(KEY_V))
    {
        if (isShift())
            eptSwitch.set();
        else
            eptSwitch.reset();
    }

    if (getKeyState(KEY_T))
    {
        if (isShift())
            locoRelease = true;
        else
            locoRelease = false;
    }
}
