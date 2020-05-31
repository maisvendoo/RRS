#include    "ep2k.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP2K::keyProcess()
{
    if (getKeyState(KEY_I))
    {
        if (isShift())
        {
            pantogrph[PANT_BWD]->setState(true);
        }
        else
        {
            pantogrph[PANT_BWD]->setState(false);
        }
    }

    if (getKeyState(KEY_O))
    {
        if (isShift())
        {
            pantogrph[PANT_FWD]->setState(true);
        }
        else
        {
            pantogrph[PANT_FWD]->setState(false);
        }
    }

    if (getKeyState(KEY_P))
    {
        if (isShift())
        {
            fast_switch->setState(true);
        }
        else
        {
            fast_switch->setState(false);
        }
    }
}


