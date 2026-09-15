#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepPanel(const double& t, const double& dt)
{
    // Регулировка яркости подсветки приборов
    auto change_intensity = [](float& intencity, float delta)
    {
        intencity = std::clamp(intencity + delta, 0.125f, 1.0f);
    };

    for (size_t cab_idx : {CAB1, CAB2})
    {
        // Контроллер машиниста
        km[cab_idx]->step(t, dt);

        // Регулировка яркости подсветки пульта (не реализовано)
        if (switchers[SWITCHER_PANEL_BRIGHTNESS][cab_idx].isSwitched(0))
        {
            change_intensity(panel_light_intensity[cab_idx], -0.5f * dt);
        }
        if (switchers[SWITCHER_PANEL_BRIGHTNESS][cab_idx].isSwitched(2))
        {
            change_intensity(panel_light_intensity[cab_idx], 0.5f * dt);
        }

        // Регулировка яркости подсветки приборов
        if (switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].isSwitched(0))
        {
            change_intensity(device_light_intensity[cab_idx], -0.5f * dt);
        }
        if (switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].isSwitched(2))
        {
            change_intensity(device_light_intensity[cab_idx], 0.5f * dt);
        }
    }


    bool tumbler_sig = tumblers[TUMBLER_SIGNAL_PANEL_BS_002][CAB1].getState() ||
                       tumblers[TUMBLER_SIGNAL_PANEL_BS_002][CAB2].getState();
    signals_module->setVoltage(Ucc * static_cast<double>(tumbler_sig));

    setSignalsModuleInputs();
    signals_module->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::setSignalsModuleInputs()
{
    // Состояние ГВ
    signals_module->setLampInputSignal(SM_GV, !main_switch->getState());

    // Состояние мотор-компрессоров
    signals_module->setLampInputSignal(SM_MK1, !motor_compressor->isPowered());
    signals_module->setLampInputSignal(SM_MK2, !motor_compressor->isPowered());

    // Состояние мотор-вертиляторов
    signals_module->setLampInputSignal(SM_V1, motor_fan[MV1]->isNoReady());
    signals_module->setLampInputSignal(SM_V2, motor_fan[MV2]->isNoReady());
    signals_module->setLampInputSignal(SM_V3, motor_fan[MV3]->isNoReady());

    signals_module->setLampInputSignal(SM_LOW_FREQ, msud->getOutputData().is_MV_low_freq);

    // Состояние тормозных цилиндров по тележкам
    for (size_t i = 0; i < brake_mech.size(); ++i)
    {
        signals_module->setLampInputSignal(SM_LAMP_TC3 + i,
                                           brake_mech[i]->getBCpressure() >
                                           msud->getOutputData().TC_min_press);
    }

    // Состояние БВ-8 тяговых двигателей
    signals_module->setLampInputSignal(SM_TD1, fast_switch[TRAC_MOTOR1]->getContactState(2));
    signals_module->setLampInputSignal(SM_TD2, fast_switch[TRAC_MOTOR2]->getContactState(2));
    signals_module->setLampInputSignal(SM_TD3, fast_switch[TRAC_MOTOR3]->getContactState(2));
    signals_module->setLampInputSignal(SM_TD4, fast_switch[TRAC_MOTOR4]->getContactState(2));
    signals_module->setLampInputSignal(SM_TD5, fast_switch[TRAC_MOTOR5]->getContactState(2));
    signals_module->setLampInputSignal(SM_TD6, fast_switch[TRAC_MOTOR6]->getContactState(2));

    // Состояние маслянного насоса тягового трансформатора
    // Так как эти насосы не моделируются - тупо по статусу тумблера "ВПОМ. МАШИНЫ"
    bool tumbler_aux = tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_AUX_MACHINES) ||
                       tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_AUX_MACHINES);
    signals_module->setLampInputSignal(SM_DM1, !tumbler_aux);
    signals_module->setLampInputSignal(SM_DM2, !tumbler_aux);
}
