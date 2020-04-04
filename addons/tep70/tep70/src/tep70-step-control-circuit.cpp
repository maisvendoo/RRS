#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepControlCircuit(double t, double dt)
{
    Ucc = max(battery->getVoltage(), 0.0);

    // Расчитываем ток, потребляемый цепями управления
    double Icc = kontaktor_fuel_pump->getCurrent() +
                 electro_fuel_pump->getCurrent() +
                 ru8->getCurrent() +
                 kontaktor_oil_pump->getCurrent() +
                 electro_oil_pump->getCurrent() +
                 oilpump_time_relay->getCurrent();

    battery->setLoadCurrent(Icc);
    battery->step(t, dt);

    // Определяем состояни цепи КТН
    bool is_KTH_on = azv_common_control.getState() &&
                     azv_fuel_pump.getState() &&
                     tumbler_disel_stop.getState();

    kontaktor_fuel_pump->setVoltage(Ucc * static_cast<double>(is_KTH_on));
    kontaktor_fuel_pump->step(t, dt);

    // Определяем состояние цепи атоматического пуска дизеля
    bool is_RU8_on = azv_common_control.getState() &&
                     km->isZero() &&
                     (button_disel_start || ru8->getContactState(0));

    ru8->setVoltage(Ucc * static_cast<double>(is_RU8_on));
    ru8->step(t, dt);

    kontaktor_oil_pump->setVoltage(Ucc * static_cast<double>(is_RU8_on));
    kontaktor_oil_pump->step(t, dt);

    oilpump_time_relay->setControlVoltage(Ucc * static_cast<double>(is_RU8_on));
    oilpump_time_relay->step(t, dt);
}
