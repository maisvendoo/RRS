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
          ru42->getCurrent() +
          mv6->getCurrent() +
          vtn->getCurrent() +
          ru4->getCurrent() +
          ru15->getCurrent() +
          rv4->getCurrent();


    battery->setChargeVoltage(starter_generator->getVoltage());
    battery->setStarterCurrent(starter_generator->getAncorCurrent());
    battery->setLoadCurrent(Icc);
    battery->step(t, dt);

    // Определяем состояни цепи контактора топливного насоса (КТН)
    bool is_KTH_on = azv_fuel_pump.getState() &&
                     tumbler_disel_stop.getState() &&
                     ru6->getContactState(1);

    kontaktor_fuel_pump->setVoltage(Ucc * static_cast<double>(is_KTH_on));
    kontaktor_fuel_pump->step(t, dt);

    // Состояние цепи кнопки "Пуск дизеля"
    bool is_Button_Start_on = azv_common_control.getState() &&
                              km->isZero() &&
                              (button_disel_start || ru8->getContactState(0));

    // Определяем состояние цепи катушки реле РУ8
    bool is_RU8_on = is_Button_Start_on &&
                     ru42->getContactState(0) &&
                     kontaktor_fuel_pump->getContactState(1);

    ru8->setVoltage(Ucc * static_cast<double>(is_RU8_on));
    ru8->step(t, dt);


    // Состояние цепи контактора маслянного насоса (КМН) при пуске
    bool is_KMH_on_start = is_Button_Start_on && ru42->getContactState(0);

    // Состояние цепи КМН при остановке дизеля (прокачка после остановки)
    bool is_KMN_on_stop = azv_common_control.getState() &&
                          ru6->getContactState(3) &&
                          ru15->getContactState(2);

    kontaktor_oil_pump->setVoltage(Ucc * static_cast<double>(is_KMH_on_start || is_KMN_on_stop));
    kontaktor_oil_pump->step(t, dt);

    // Состояние цепи реле времени РВ3 (задает время предпусковой прокачки)
    bool is_RV3_on = is_Button_Start_on && ru42->getContactState(0);

    oilpump_time_relay->setControlVoltage(Ucc * static_cast<double>(is_RV3_on));
    oilpump_time_relay->step(t, dt);

    // Проверяем давление масла
    bool is_RDM4_on = static_cast<bool>(hs_p(disel->getOilPressure() - 0.05));


    bool is_MV6_on = azv_fuel_pump.getState() &&
                     tumbler_disel_stop.getState() &&
                    (ru8->getContactState(1) || is_RDM4_on);

    mv6->setVoltage(Ucc * static_cast<double>(is_MV6_on));
    mv6->step(t, dt);

    // Состояние цепи контактора пуска (КД)
    bool is_KD_on = ru42->getContactState(0) &&
                    oilpump_time_relay->getContactState(0) &&
                    is_RDM4_on;

    kontaktor_starter->setVoltage(Ucc * static_cast<double>(is_KD_on));
    kontaktor_starter->step(t, dt);

    starter_time_relay->setControlVoltage(Ucc * static_cast<double>(is_KD_on));
    starter_time_relay->step(t, dt);    

    // Проверяем обороты дизеля
    bool is_RU10_on = static_cast<bool>(hs_p(disel->getOmega() - 31.4)) &&
                      ru8->getContactState(2);

    ru10->setVoltage(Ucc * static_cast<double>(is_RU10_on));
    ru10->step(t, dt);

    bool is_RU6_on = ( (ru8->getContactState(3) || is_RDM4_on) &&
                     (ru10->getContactState(0) || ru6->getContactState(0)) ) ||
                     starter_time_relay->getContactState(0);

    ru6->setVoltage(Ucc * static_cast<double>(is_RU6_on));
    ru6->step(t, dt);

    bool is_VTN_on = ru6->getContactState(0) && ru4->getContactState(0);

    vtn->setVoltage(Ucc * static_cast<double>(is_VTN_on));
    vtn->step(t, dt);

    bool is_RU4_on = km->isMoreFirst();

    ru4->setVoltage(Ucc * static_cast<double>(is_RU4_on));
    ru4->step(t, dt);

    ru42->setVoltage(Ucc * static_cast<double>(is_RU6_on));
    ru42->step(t, dt);

    bool is_RU15_on = azv_common_control.getState() &&
                      rv4->getContactState(0) &&
                      (ru6->getContactState(2) || ru15->getContactState(0));

    ru15->setVoltage(Ucc * static_cast<double>(is_RU15_on));
    ru15->step(t, dt);

    bool is_RV4_on = azv_common_control.getState() &&
                     ru6->getContactState(3) &&
                     ru15->getContactState(1);

    rv4->setControlVoltage(Ucc * static_cast<double>(is_RV4_on));
    rv4->step(t, dt);
}
