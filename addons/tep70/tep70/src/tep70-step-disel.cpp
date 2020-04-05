#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepDisel(double t, double dt)
{
    disel->setQ_emn(electro_oil_pump->getOilFlow());
    disel->setStarterTorque(starter_generator->getTorque());
    disel->setFuelPressure(electro_fuel_pump->getFuelPressure());
    disel->setMV6state(mv6->getContactState(0));
    disel->step(t, dt);

    starter_generator->setAncorVoltage( battery->getVoltage() *  static_cast<double>(kontaktor_starter->getContactState(0)));
    starter_generator->setLoadCurrent(Icc);
    starter_generator->setOmega(disel->getStarterOmega());
    starter_generator->step(t, dt);
}
