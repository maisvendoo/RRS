#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::debugOutput(double t, double dt)
{
    DebugMsg = QString("t: %1 ЗР: %2 КД: %3 Обороты: %4 Uвозб.: %5")
            .arg(t, 10, 'f', 2)
            .arg(zr->getPressure(), 4, 'f',2)
            .arg(static_cast<int>(kontaktor_starter->getContactState(1)), 1)
            .arg(disel->getOmega() * 30.0 / Physics::PI, 7, 'f', 1)
            .arg(field_gen->getVoltage(), 7, 'f', 1);
}


