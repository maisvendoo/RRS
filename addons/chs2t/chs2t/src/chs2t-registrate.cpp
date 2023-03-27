#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::registrate(double t, double dt)
{
    if (reg == nullptr)
        return;

    reg->print(motor->getDebugMsg(), t, dt);
}
