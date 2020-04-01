#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepControlCircuit(double t, double dt)
{
    Ucc = max(battery->getVoltage(), 0.0);

    // Расчитываем ток, потребляемый цепями управления
    double Icc = kontaktor_fuel_pump->getCurrent() +
                 electro_fuel_pump->getCurrent();

    battery->setLoadCurrent(Icc);
    battery->step(t, dt);

    // Определяем состояни цепи КТН
    bool is_KTH_on = azv_common_control.getState() &&
                     azv_fuel_pump.getState() &&
                     tumbler_disel_stop.getState();

    kontaktor_fuel_pump->setVoltage(Ucc * static_cast<double>(is_KTH_on));
    kontaktor_fuel_pump->step(t, dt);
}
