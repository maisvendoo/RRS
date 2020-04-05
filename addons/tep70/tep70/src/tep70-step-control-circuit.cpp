#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepControlCircuit(double t, double dt)
{
    Ucc = max(battery->getVoltage(), starter_generator->getVoltage());

    // Расчитываем ток, потребляемый цепями управления
    Icc = kontaktor_fuel_pump->getCurrent() +
          electro_fuel_pump->getCurrent() +
          ru8->getCurrent() +
          kontaktor_oil_pump->getCurrent() +
          electro_oil_pump->getCurrent() +
          oilpump_time_relay->getCurrent() +
          kontaktor_starter->getCurrent() +
          starter_time_relay->getCurrent() +
          ru10->getCurrent() +
          ru6->getCurrent() +
          ru42->getCurrent();


    battery->setChargeVoltage(starter_generator->getVoltage());
    battery->setStarterCurrent(starter_generator->getAncorCurrent());
    battery->setLoadCurrent(Icc);
    battery->step(t, dt);

    // Определяем состояни цепи КТН
    bool is_KTH_on = azv_common_control.getState() &&
                     azv_fuel_pump.getState() &&
                     tumbler_disel_stop.getState() &&
                     ru6->getContactState(1);

    kontaktor_fuel_pump->setVoltage(Ucc * static_cast<double>(is_KTH_on));
    kontaktor_fuel_pump->step(t, dt);

    // Определяем состояние цепи атоматического пуска дизеля
    bool is_RU8_on = azv_common_control.getState() &&
                     km->isZero() &&
                     (button_disel_start || ru8->getContactState(0)) &&
                     ru42->getContactState(0);

    ru8->setVoltage(Ucc * static_cast<double>(is_RU8_on));
    ru8->step(t, dt);

    kontaktor_oil_pump->setVoltage(Ucc * static_cast<double>(is_RU8_on && ru42->getContactState(0)));
    kontaktor_oil_pump->step(t, dt);

    oilpump_time_relay->setControlVoltage(Ucc * static_cast<double>(is_RU8_on));
    oilpump_time_relay->step(t, dt);

    kontaktor_starter->setVoltage(Ucc * static_cast<double>(oilpump_time_relay->getContactState(0)));
    kontaktor_starter->step(t, dt);

    starter_time_relay->setControlVoltage(Ucc * static_cast<double>(oilpump_time_relay->getContactState(1)));
    starter_time_relay->step(t, dt);


    // Проверяем давление масла
    bool is_RDM4_on = static_cast<bool>(hs_p(disel->getOilPressure() - 0.05));

    // Проверяем обороты дизеля
    bool is_RU10_on = static_cast<bool>(hs_p(disel->getOmega() - 31.4)) &&
                      ru8->getContactState(2);

    ru10->setVoltage(Ucc * static_cast<double>(is_RU10_on));
    ru10->step(t, dt);

    bool is_RU6_on = is_RDM4_on &&
                     (ru10->getContactState(0) || starter_time_relay->getContactState(0) || ru6->getContactState(0));

    ru6->setVoltage(Ucc * static_cast<double>(is_RU6_on));
    ru6->step(t, dt);

    ru42->setVoltage(Ucc * static_cast<double>(is_RU6_on));
    ru42->step(t, dt);
}
