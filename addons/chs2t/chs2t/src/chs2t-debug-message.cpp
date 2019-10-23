#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(dt)

    DebugMsg = QString("t = %1 V = %2")
        .arg(t, 10, 'f', 1)
        .arg(velocity*Physics::kmh, 4, 'f', 1);
}
