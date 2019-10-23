#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::keyProcess()
{
    if (getKeyState(KEY_8))
    {
        if (isShift())
            mk_tumbler.set();

        else
            mk_tumbler.reset();
    }

    if (getKeyState(KEY_9))
    {
        if (isShift())
            EDTSwitch.set();
        else
            EDTSwitch.reset();
    }
}
