#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepFuelSystem(double t, double dt)
{
    fuel_tank->setFuelConsumption(0.0);
    fuel_tank->step(t, dt);

    // Пересчитываем массу тепловоза, с учетом текущем массы топлива в баке
    full_mass = empty_mass +
                payload_mass * payload_coeff +
                fuel_tank->getFuelMass();

    electro_fuel_pump->setVoltage(static_cast<double>(Ucc * kontaktor_fuel_pump->getContactState(0)));
    electro_fuel_pump->setFuelLevel(fuel_tank->getFuelLevel());
    electro_fuel_pump->step(t, dt);
}
