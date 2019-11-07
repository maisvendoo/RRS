#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::registrate(double t, double dt)
{
    if (reg == nullptr)
        return;

    QString msg = QString("%1 %2 %3 %4 %5")
            .arg(t)
            .arg(velocity * Physics::kmh)
            .arg(abs(generator->getIa()))
            .arg(BrakeReg->getU())
            .arg(pulseConv->getUf());

    reg->print(msg, t, dt);
}
