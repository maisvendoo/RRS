#include    "passcar.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::stepRegistrator(double t, double dt)
{
    QString line = QString("%1 %2 %3 %4")
            .arg(t, 10, 'f', 1)
            .arg(brakepipe->getPressure(), 7, 'f', 3)
            .arg(supply_reservoir->getPressure(), 7, 'f', 3)
            .arg(brake_mech->getBCpressure(), 7, 'f', 3);

    registrator->print(line, t, dt);
}
