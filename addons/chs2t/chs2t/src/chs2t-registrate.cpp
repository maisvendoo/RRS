#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::registrate(double t, double dt)
{
    QString msg = QString("%1 %2 %3 %4")
            .arg(t)
            .arg(velocity * Physics::kmh)
            .arg(tracForce_kN)
            .arg(motor->getIa());

    reg->print(msg, t, dt);
}
