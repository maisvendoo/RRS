#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::debugOutput(double t, double dt)
{
    DebugMsg = QString("t: %1 КТН: %2 Давление топлива: %3")
            .arg(t, 10, 'f', 2)
            .arg(static_cast<int>(kontaktor_fuel_pump->getContactState(0)), 2)
            .arg(electro_fuel_pump->getFuelPressure(), 4, 'f', 2);
}
