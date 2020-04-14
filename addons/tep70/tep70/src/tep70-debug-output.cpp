#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::debugOutput(double t, double dt)
{
    Q_UNUSED(dt)

    DebugMsg = QString("t: %1 x: %2 ЗР: %3 n: %4 Тяга: %5 Uген: %6 Iген: %7 Pген: %8 Расх. топл.: %9 Топл.: %10")
            .arg(t, 10, 'f', 2)
            .arg(railway_coord / 1000.0, 7, 'f', 2)
            .arg(zr->getPressure(), 4, 'f',2)
            .arg(disel->getOmega() * 30.0 / Physics::PI, 7, 'f', 1)
            .arg(tracForce / 1000.0, 7, 'f', 1)
            .arg(trac_gen->getVoltage(), 6, 'f', 1)
            .arg(I_gen, 8, 'f', 1)
            .arg(trac_gen->getVoltage() * I_gen / 1000.0, 6, 'f', 1)
            .arg(disel->getFuelFlow(), 6, 'f', 4)
            .arg(fuel_tank->getFuelMass(), 6, 'f', 1);
}


