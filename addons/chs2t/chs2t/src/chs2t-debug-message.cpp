#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(dt)

    DebugMsg = QString("t: %1 x: %2 v: %3 ТМ: %4 ЗР: %5 ТЦ: %6")
        .arg(t, 10, 'f', 1)
        .arg(railway_coord / 1000.0, 5, 'f', 1)
        .arg(velocity * Physics::kmh, 5, 'f', 1)
        .arg(pTM, 5, 'f', 2)
        .arg(spareReservoir->getPressure(), 5, 'f', 2)
        .arg(brakesMech[1]->getBrakeCylinderPressure(), 5, 'f', 2);
}
