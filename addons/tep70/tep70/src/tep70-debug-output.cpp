#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::debugOutput(double t, double dt)
{
    DebugMsg = QString("t: %1 КТН: %2 Давл. топ.: %3 КД: %4 РУ6: %5 РВ1: %6 Давл. масл.: %7 Расх. топл.: %8 Обороты: %9")
            .arg(t, 10, 'f', 2)
            .arg(static_cast<int>(kontaktor_fuel_pump->getContactState(0)), 2)
            .arg(electro_fuel_pump->getFuelPressure(), 4, 'f', 2)
            .arg(static_cast<int>(kontaktor_starter->getContactState(0)), 2)
            .arg(static_cast<int>(ru6->getContactState(0)), 2)
            .arg(static_cast<int>(starter_time_relay->getContactState(0)), 2)
            .arg(disel->getOilPressure(), 4, 'f', 2)
            .arg(disel->getFuelFlow(), 6, 'f', 3)
            .arg(disel->getOmega() * 30.0 / Physics::PI, 6, 'f', 1);
}
