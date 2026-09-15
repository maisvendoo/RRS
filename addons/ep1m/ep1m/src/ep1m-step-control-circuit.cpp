#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepControlCircuit(const double& t, const double& dt)
{
    // Цепь питания промежуточного реле KV44
    bool tumbler_msud = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_MSUD) ||
                        tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_MSUD);
    bool is_kv44_on = tumbler_msud;

    kv44->setVoltage(Ucc * static_cast<double>(is_kv44_on));
    kv44->step(t, dt);

    // Цепь питания промежуточного реле KV39
    bool tumbler_pant1 = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_PANT1) ||
                         tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_PANT2);
    bool tumbler_pant2 = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_PANT2) ||
                         tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_PANT1);
    bool is_kv39_on = tumbler_pant1 || tumbler_pant2;

    kv39->setVoltage(Ucc * static_cast<double>(is_kv39_on));
    kv39->step(t, dt);

    // Цепь питания промежуточных реле КV21 и KV22
    bool km_15_16 = (km[CAB1]->isContacts15_16() || km[CAB2]->isContacts15_16());
    bool is_kv21_on = km_15_16;
    bool is_kv22_on = km_15_16;

    kv21->setVoltage(Ucc * static_cast<double>(is_kv21_on));
    kv21->step(t, dt);

    kv22->setVoltage(Ucc * static_cast<double>(is_kv22_on));
    kv22->step(t, dt);

    // Цепь питания реле KV23
    bool is_kv23_on = is_kv22_on;
    kv23->setVoltage(Ucc * is_kv23_on);
    kv23->step(t, dt);

    // Цепь питания контактора КМ43
    bool is_km43_on = tumbler_msud &&
                      (kv21->getContactState(1) || km43->getContactState(0));

    km43->setVoltage(Ucc * static_cast<double>(is_km43_on));
    km43->step(t, dt);

    // Цепь на проводе Н211
    bool tumbler_m_s = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_MAIN_SWITCH) ||
                       tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_MAIN_SWITCH);
    bool tumbler_r_p = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_RETURN_PROTECTION) ||
                       tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_RETURN_PROTECTION);
    bool button_off = tumblers[BUTTON_MAIN_SWITCH_OFF][CAB1].getState() ||
                      tumblers[BUTTON_MAIN_SWITCH_OFF][CAB2].getState();
    is_N211_on = tumbler_m_s && tumbler_r_p && !button_off;

    is_N212_on = is_N211_on && kv21->getContactState(0);

    bool is_kv41_on = (is_N212_on && kv41->getContactState(0)) ||
                      (is_N212_on && main_switch->getState());

    kv41->setVoltage(Ucc * static_cast<double>(is_kv41_on));
    kv41->step(t, dt);

    // Состояние цепи "Возврат защиты ГВ"
    return_GV =
            is_N211_on &&
            kv41->getContactState(1) &&
            kv39->getContactState(1) &&
            kv44->getContactState(3) &&
            kv23->getContactState(0);

    main_switch->setReturn(return_GV);

    stepTractionControl(t, dt);

    stepRecuperationControl(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EP1m::getHoldingCoilState()
{
    bool tumbler_m_s = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_MAIN_SWITCH) ||
                       tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_MAIN_SWITCH);
    bool button_off = tumblers[BUTTON_MAIN_SWITCH_OFF][CAB1].getState() ||
                       tumblers[BUTTON_MAIN_SWITCH_OFF][CAB2].getState();
    bool is_holding_coil_on = tumbler_m_s && !button_off &&
                              kv44->getContactState(0) &&
                              kv39->getContactState(0) &&
                              safety_valve->getState();

    return is_holding_coil_on;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepTractionControl(const double& t, const double& dt)
{
    bool km_5_6 = (km[CAB1]->isContacts5_6() || km[CAB2]->isContacts5_6());
    bool epk_emergency = (epk[CAB1]->getEmergencyBrakeContact() || epk[CAB2]->getEmergencyBrakeContact());
    bool is_KV11_KV12_on = km_5_6 && epk_emergency;

    kv11->setVoltage(Ucc * static_cast<double>(is_KV11_KV12_on));
    kv11->step(t, dt);

    kv12->setVoltage(Ucc * static_cast<double>(is_KV11_KV12_on));
    kv12->step(t, dt);

    // Контроль давления в тормозной магистрали
    sp4->setValue(brakepipe->getPressure());

    // Кнопка "Экстренное торможение" на пульте помощника
    bool button_emerg = tumblers[BUTTON_EMERGENCY_BRAKE][CAB1].getState() ||
                        tumblers[BUTTON_EMERGENCY_BRAKE][CAB2].getState();
    // Контакт экстренного торможения на кране машиниста
    bool is_SQ3 = (brake_lock[CAB1]->isStateOn() && (brake_crane[CAB1]->getPositionName() == "VI")) ||
                  (brake_lock[CAB2]->isStateOn() && (brake_crane[CAB2]->getPositionName() == "VI"));

    bool is_KV13_on = km_5_6 &&
                      (!is_SQ3) &&
                      (!button_emerg);

    kv13->setVoltage(Ucc * static_cast<double>(is_KV13_on));
    kv13->step(t, dt);

    // Разрешение тяги от КЛУБ
    kv84->setVoltage(Ucc * static_cast<double>(klub_BEL->isTractionAllowed()));
    kv84->step(t, dt);

    // Включение реле времени KT10
    bool km_13_14 = (km[CAB1]->isContacts13_14() || km[CAB2]->isContacts13_14());
    bool km_1_2 = (km[CAB1]->isContacts1_2() || km[CAB2]->isContacts3_4());
    bool km_3_4 = (km[CAB1]->isContacts3_4() || km[CAB2]->isContacts1_2());

    bool is_N3_on = km_13_14 &&
                    km_1_2 &&
                    reversor->isForward();

    bool is_N4_on = km_13_14 &&
                    km_3_4 &&
                    reversor->isBackward();

    kt10->setControlVoltage(Ucc * static_cast<double>(is_N3_on || is_N4_on));
    kt10->step(t, dt);

    // Включение реле KV14
    kv14->setVoltage(Ucc * static_cast<double>(msud->getOutputData().kv14_On));
    kv14->step(t, dt);

    // Включение реле KV15
    bool is_N39_on = is_H36 &&
            qt1->getContactState(2) &&
            qt1->getContactState(3);

    is_N53_on = is_N45_on;

    for (size_t i = 0; i < fast_switch.size(); ++i)
    {
        is_N53_on = is_N53_on && fast_switch[i]->getContactState(3);
    }

    bool is_N54_on = is_N53_on &&
            km14->getContactState(1) &&
            sp3->getClosedContactState();

    sp3->setInputPressure(brake_mech[TROLLEY_FWD]->getBCpressure());
    sp3->step(t, dt);

    bool is_N40_on = (is_N39_on || is_N54_on) && kt10->getContactState(0);

    bool is_KV15_on = is_N40_on &&
            ( (kt1->getContactState(1) && kv15->getContactState(1)) ||
               kv22->getContactState(1) );

    kv15->setVoltage(Ucc * static_cast<double>(is_KV15_on));
    kv15->step(t, dt);

    // Включение контактора KM41
    bool is_H153 = kv23->getContactState(0) || km41->getContactState(0);

    bool is_KM41_on = is_H153 &&
            ( (kv15->getContactState(2) && qt1->getContactState(7)) ||
              kt5->getContactState(0));

    km41->setVoltage(Ucc * static_cast<double>(is_KM41_on));
    km41->step(t, dt);

    // Включение контактора KM42
    bool is_H163 = kv23->getContactState(1) || km42->getContactState(0);

    bool is_KM42_on = is_H163 &&
            ( (kv15->getContactState(3) && qt1->getContactState(8)) ||
              kt5->getContactState(1));

    km42->setVoltage(Ucc * static_cast<double>(is_KM42_on));
    km42->step(t, dt);

    // Запуск реле времени КТ1
    bool is_KT1_on = km41->getContactState(1) && km42->getContactState(1);

    kt1->setControlVoltage(Ucc * static_cast<double>(is_KT1_on));
    kt1->step(t, dt);

    // Включение БВ от ВИП1 на ТЭД1, ТЭД2, ТЭД3
    bool is_Hold_1_3 = is_H153 && (main_switch->getState());


    fast_switch[TRAC_MOTOR1]->setHold(is_Hold_1_3);
    fast_switch[TRAC_MOTOR2]->setHold(is_Hold_1_3);
    fast_switch[TRAC_MOTOR3]->setHold(is_Hold_1_3);

    bool is_Power_On_1_3 = is_N212_on;

    fast_switch[TRAC_MOTOR1]->setPowerOn(is_Power_On_1_3);
    fast_switch[TRAC_MOTOR2]->setPowerOn(is_Power_On_1_3);
    fast_switch[TRAC_MOTOR3]->setPowerOn(is_Power_On_1_3);

    // Включение БВ от ВИП2 на ТЭД4, ТЭД5, ТЭД6
    bool is_Hold_4_6 = is_H163 && (main_switch->getState());

    fast_switch[TRAC_MOTOR4]->setHold(is_Hold_4_6);
    fast_switch[TRAC_MOTOR5]->setHold(is_Hold_4_6);
    fast_switch[TRAC_MOTOR6]->setHold(is_Hold_4_6);

    bool is_Power_On_4_6 = is_N212_on;

    fast_switch[TRAC_MOTOR4]->setPowerOn(is_Power_On_4_6);
    fast_switch[TRAC_MOTOR5]->setPowerOn(is_Power_On_4_6);
    fast_switch[TRAC_MOTOR6]->setPowerOn(is_Power_On_4_6);

    bool circuit_state = true;
    for (size_t i = 0; i < fast_switch.size(); ++i)
    {
        circuit_state = circuit_state && (!fast_switch[i]->getContactState(2));
    }

    circuit_state = circuit_state &&
            km41->getContactState(0) &&
            km42->getContactState(0);

    msud_input.is_traction = circuit_state;

    msud_input.is_brake = circuit_state && k1->getContactState(1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepRecuperationControl(const double& t, const double& dt)
{
    bool km_11_12 = (km[CAB1]->isContacts11_12() || km[CAB2]->isContacts11_12());
    is_N45_on = km_11_12;

    // Цепь подготовки реле КТ4
    bool is_KT4_on = is_N45_on &&
            km41->getContactState(4) &&
            km42->getContactState(4);

    kt4->setControlVoltage(Ucc * static_cast<double>(is_KT4_on));

    kt4->step(t, dt);

    // Цепь контактора вертилятора ББР
    bool tumbler_aux = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_AUX_MACHINES) ||
                       tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_AUX_MACHINES);
    bool is_KM14_on = tumbler_aux &&
                      qt1->getContactState(4) &&
                      kt10->getContactState(4);

    km14->setVoltage(Ucc * static_cast<double>(is_KM14_on));
    km14->step(t, dt);


    // Цепь подготовки реле КТ5
    bool is_N55_on = is_N53_on && kv15->getContactState(4);

    bool is_KT5_on = is_N55_on;

    kt5->setControlVoltage(Ucc * static_cast<double>(is_KT5_on));

    kt5->step(t, dt);

    // Цепь подготовки контактора К1
    bool is_N57_on = is_N55_on && kt4->getContactState(0);

    k1->setVoltage(Ucc * static_cast<double>(is_N57_on));

    k1->step(t, dt);

    sp6->setInputPressure(brakepipe->getPressure());
    sp6->step(t, dt);

    bool release_brakes = (tumblers[BUTTON_RELEASE_BRAKES][CAB1].getState() ||
                           tumblers[BUTTON_RELEASE_BRAKES][CAB2].getState());
    bool km_5_6 = (km[CAB1]->isContacts5_6() || km[CAB2]->isContacts5_6());

    bool is_Y3_on = (is_N55_on && kv21->getContactState(3) && sp6->getOpenContactState()) ||
                    (release_brakes && km_5_6);

    Y3->setVoltage(Ucc * static_cast<double>(is_Y3_on));

    bool is_Y4_on = is_N45_on &&
            kv21->getContactState(4) &&
            k1->getContactState(2);

    Y4->setVoltage(Ucc * static_cast<double>(!is_Y4_on));
}
