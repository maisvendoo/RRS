#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepDisel(double t, double dt)
{
    disel->setQ_emn(electro_oil_pump->getOilFlow());
    disel->setStarterTorque(starter_generator->getTorque());
    disel->setFGTorque(field_gen->getTorque());
    disel->setGenTorque(trac_gen->getTorque());
    disel->setFuelPressure(electro_fuel_pump->getFuelPressure());
    disel->setMV6state(mv6->getContactState(0));
    disel->setVTNstate(vtn->getContactState(0));
    disel->setRefFreq(km->getRefFreq());
    disel->step(t, dt);

    double I_gen = Icc + motor_compressor->getCurrent();

    starter_generator->setAncorVoltage( battery->getVoltage() *  static_cast<double>(kontaktor_starter->getContactState(0)));
    starter_generator->setFieldVoltage(voltage_regulator->getFieldVoltage() * static_cast<double>(krn->getContactState(0)));
    starter_generator->setLoadCurrent(I_gen);
    starter_generator->setOmega(disel->getStarterOmega());
    starter_generator->setMotorMode(krn->getContactState(2));
    starter_generator->step(t, dt);

    voltage_regulator->setBatteryVoltage(battery->getVoltage());
    voltage_regulator->setRefVoltage(110.0);
    voltage_regulator->setVoltage(starter_generator->getVoltage());
    voltage_regulator->step(t, dt);
}
