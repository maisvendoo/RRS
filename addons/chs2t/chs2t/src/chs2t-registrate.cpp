#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::registrate(double t, double dt)
{
    if (reg == nullptr)
        return;

    QString msg = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9")
            .arg(t)
            .arg(railway_coord / 1000.0)
            .arg(velocity * Physics::kmh)
            .arg(energy_counter->getFullEnergy())
            .arg(energy_counter->getResistorEnergy())
            .arg(energy_counter->getTracEnergy())
            .arg(energy_counter->getFullPower())
            .arg(energy_counter->getResistorPower())
            .arg(energy_counter->getTracPower());

    reg->print(msg, t, dt);
}
