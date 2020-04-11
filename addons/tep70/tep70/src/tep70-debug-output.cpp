#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::debugOutput(double t, double dt)
{
    DebugMsg = QString("t: %1 ЗР: %2 КД: %3 Обороты: %4 Тяга: %5 Uген: %6 Iген: %7 Pген: %10 Uвг: %8 E: %9")
            .arg(t, 10, 'f', 2)
            .arg(zr->getPressure(), 4, 'f',2)
            .arg(static_cast<int>(kontaktor_starter->getContactState(1)), 1)
            .arg(disel->getOmega() * 30.0 / Physics::PI, 7, 'f', 1)
            .arg(tracForce / 1000.0, 7, 'f', 1)
            .arg(trac_gen->getVoltage(), 6, 'f', 1)
            .arg(I_gen, 8, 'f', 1)
            .arg(field_reg->getFieldVoltage(), 6, 'f', 1)
            .arg(motor[0]->getEMF(), 6, 'f', 1)
            .arg(trac_gen->getVoltage() * I_gen / 1000.0, 6, 'f', 1);
}


