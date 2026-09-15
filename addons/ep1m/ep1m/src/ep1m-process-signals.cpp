#include    "ep1m.h"
#include    <QTime>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::signalsOutput(const simulator_time_t& t, const double& dt)
{
    (void) dt;

    analogSignal[SERIAL_NUMBER] = 384.0f;

    analogSignal[WHEELSET_1] = TO_FLOAT(wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_2] = TO_FLOAT(wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_3] = TO_FLOAT(wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_4] = TO_FLOAT(wheel_rotation_angle[3] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_5] = TO_FLOAT(wheel_rotation_angle[4] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_6] = TO_FLOAT(wheel_rotation_angle[5] / 2.0 / Physics::PI);

    // Состояние главного выключателя
    analogSignal[MAIN_SWITCH_POS] = static_cast<float>(main_switch->getKnifePos());

    // Состояние токоприемников
    analogSignal[PANT1_POS] = static_cast<float>(pant[PANT1]->getHeight());
    analogSignal[PANT2_POS] = static_cast<float>(pant[PANT2]->getHeight());

    analogSignal[SIGNAL_DATE] = TO_FLOAT((t.date.year() - 2000) * 1024 +
                                         t.date.month() * 32 +
                                         t.date.day());
    analogSignal[SIGNAL_TIME] = TO_FLOAT(t.time.hour() * 3600 +
                                         t.time.minute() * 60 +
                                         t.time.sec());

    // Кабины
    for (auto cab_idx : {CAB1, CAB2})
    {
        std::uint16_t d = (SPOTLIGHT_BWD - SPOTLIGHT_FWD) * cab_idx;

        // Освещение
        bool is_power = (Ucc > 0.9 * 50.0);
        if (is_power)
        {
            // Прожектор
            if (tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].getState())
            {
                const std::uint8_t intensity = 1 + tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].getState();
                analogSignal[SPOTLIGHT_FWD + d] = static_cast<float>(intensity) / 2.0f;
            }
            else
            {
                analogSignal[SPOTLIGHT_FWD + d] = 0.0f;
            }

            // Левый буферный
            if (tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].getState())
            {
                analogSignal[BUFFERLIGHT_FWD_L_WHITE + d] = static_cast<float>(tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].getState());
                analogSignal[BUFFERLIGHT_FWD_L_RED + d] = static_cast<float>(!tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].getState());
            }
            else
            {
                analogSignal[BUFFERLIGHT_FWD_L_WHITE + d] = 0.0f;
                analogSignal[BUFFERLIGHT_FWD_L_RED + d] = 0.0f;
            }

            // Правый буферный
            if (tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].getState())
            {
                analogSignal[BUFFERLIGHT_FWD_R_WHITE + d] = static_cast<float>(tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].getState());
                analogSignal[BUFFERLIGHT_FWD_R_RED + d] = static_cast<float>(!tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].getState());
            }
            else
            {
                analogSignal[BUFFERLIGHT_FWD_R_WHITE + d] = 0.0f;
                analogSignal[BUFFERLIGHT_FWD_R_RED + d] = 0.0f;
            }

            // Освещение кабины
            if (tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].getState())
            {
                const std::uint8_t intensity = 1 + tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].getState();
                if (tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].getState())
                {
                    analogSignal[CAB1_LIGHT_CABINE + d] = 0.0f;
                    analogSignal[CAB1_GREEN_CABINE + d] = static_cast<float>(intensity) / 2.0f;
                }
                else
                {
                    analogSignal[CAB1_LIGHT_CABINE + d] = static_cast<float>(intensity) / 2.0f;
                    analogSignal[CAB1_GREEN_CABINE + d] = 0.0f;
                }
            }
            else
            {
                analogSignal[CAB1_LIGHT_CABINE + d] = 0.0f;
                analogSignal[CAB1_GREEN_CABINE + d] = 0.0f;
            }

            // Подсветка приборов
            if (tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].getState())
            {
                analogSignal[CAB1_LIGHT_DEVICES + d] = device_light_intensity[cab_idx];
            }
            else
            {
                analogSignal[CAB1_LIGHT_DEVICES + d] = 0.0f;
            }
        }

        // Циферблаты
        analogSignal[CAB1_PRESSURE_ER + d] = TO_FLOAT(brake_crane[cab_idx]->getERpressure() / 1.0);
        analogSignal[CAB1_PRESSURE_BP + d] = TO_FLOAT(brakepipe->getPressure() / 1.0);
        analogSignal[CAB1_PRESSURE_FL + d] = TO_FLOAT(main_reservoir->getPressure() / 1.6);
        analogSignal[CAB1_PRESSURE_BC + d] = TO_FLOAT(brake_mech[TROLLEY_FWD]->getBCpressure() / 1.6);
        analogSignal[CAB1_ENGINE_CURRENT + d] = TO_FLOAT(abs(trac_motor[TRAC_MOTOR1]->getAncorCurrent()) / 1500.0);
        analogSignal[CAB1_WIRE_VOLTAGE + d] = TO_FLOAT(main_switch->getU_out() / 30000.0);
        analogSignal[CAB1_EPB_CURRENT + d] = TO_FLOAT(epb_converter->getOutputCurrent() / 10.0);
        double voltage = tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].getState() ?
                             epb_converter->getOutputVoltage() : battery->getVoltage();
        analogSignal[CAB1_EPB_VOLTAGE + d] = TO_FLOAT(voltage / 150.0);

        // Дверца тумбы с устройством блокировки тормозов
        analogSignal[CAB1_OPEN_BRAKELOCK_DOOR + d] = static_cast<float>(brake_lock_door[cab_idx].getState());
        // Рукоятка УБТ, комбинированный кран, поездной кран, локомотивный кран
        analogSignal[CAB1_UBT_IS_KEY_HANDLE + d] = static_cast<float>(brake_lock[cab_idx]->isLockHandle());
        analogSignal[CAB1_UBT_KEY_HANDLE_POS + d] = static_cast<float>(brake_lock[cab_idx]->getLockHandlePosition());
        analogSignal[CAB1_UBT_COMBINE_CRANE_POS + d] = static_cast<float>(brake_lock[cab_idx]->getCombineCraneHandlePosition());
        analogSignal[CAB1_BRAKE_CRANE_HANDLE_POS + d] = static_cast<float>(brake_crane[cab_idx]->getHandlePosition());
        analogSignal[CAB1_LOCO_CRANE_HANDLE_POS + d] = static_cast<float>(loco_crane[cab_idx]->getHandlePosition());

        // ЭПК
        analogSignal[CAB1_AUTOSTOP_IS_KEY + d] = static_cast<float>(epk[cab_idx]->isKey());
        analogSignal[CAB1_AUTOSTOP_KEY_POS + d] = static_cast<float>(epk[cab_idx]->isKeyOn());

        // Контроллер машиниста
        analogSignal[CAB1_IS_REVERS_HANDLE + d] = TO_FLOAT(km[cab_idx]->isReversHandle());
        analogSignal[CAB1_REVERS_HANDLE_POS + d] = km[cab_idx]->getReversHandlePos();
        analogSignal[CAB1_KM_MAIN_HANDLE_POS + d] = km[cab_idx]->getHandlePosition();
        analogSignal[CAB1_REF_SPEED_POS + d] = TO_FLOAT(km[cab_idx]->getRefSpeedLevel());

        // Панель тумблеров с ключом, на столешнице по центру
        analogSignal[CAB1_IS_TUMBLERS_KEY + d] = TO_FLOAT(tumblers_panel[cab_idx]->isKey());
        analogSignal[CAB1_TUMBLERS_KEY_STATE + d] = TO_FLOAT(tumblers_panel[cab_idx]->isKeyOn());
        analogSignal[CAB1_TUMBLER_MSUD + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_MSUD));
        analogSignal[CAB1_TUMBLER_LOCK_VVK + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_LOCK_VVK));
        analogSignal[CAB1_TUMBLER_PANT1 + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_PANT1));
        analogSignal[CAB1_TUMBLER_PANT2 + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_PANT2));
        analogSignal[CAB1_TUMBLER_RETURN_PROTECTION + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_RETURN_PROTECTION));
        analogSignal[CAB1_TUMBLER_MAIN_SWITCH + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_MAIN_SWITCH));
        analogSignal[CAB1_TUMBLER_AUX_DEVICES + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_AUX_MACHINES));
        analogSignal[CAB1_TUMBLER_COMPRESSOR + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_COMPRESSOR));
        analogSignal[CAB1_TUMBLER_FAN1 + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_MOTOR_FAN1));
        analogSignal[CAB1_TUMBLER_FAN2 + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_MOTOR_FAN2));
        analogSignal[CAB1_TUMBLER_FAN3 + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_MOTOR_FAN3));
        analogSignal[CAB1_TUMBLER_EPB + d] = TO_FLOAT(tumblers_panel[cab_idx]->getTumblerState(EP1MTumblersPanel::TUMBLER_EPT));

        // Панель на столешнице справа
        analogSignal[CAB1_BUTTON_WHISTLE + d] = TO_FLOAT(horn[cab_idx]->isSvistok());
        analogSignal[CAB1_BUTTON_TYPHON + d] = TO_FLOAT(horn[cab_idx]->isTifon());
        analogSignal[CAB1_BUTTON_SAND + d] = TO_FLOAT(sand_system->isSandDelivery());
        analogSignal[CAB1_TOOGLE_AUTOSAND + d] = TO_FLOAT(tumblers[TUMBLER_AUTOSAND][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_MPK_1_OR_2 + d] = TO_FLOAT(tumblers[TUMBLER_MPK][cab_idx].getState());
        analogSignal[CAB1_BUTTON_RETURN_AUX_PROTECTION + d] = TO_FLOAT(tumblers[BUTTON_RETURN_AUX_PROTECTION][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_SAND_ON_EMERGENCY + d] = TO_FLOAT(tumblers[TUMBLER_SAND_ON_EMERGENCY][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_MANUAL_OR_AUTO + d] = TO_FLOAT(tumblers[TUMBLER_AUTO_MODE][cab_idx].getState());

        // Панель на средней тумбе
        analogSignal[CAB1_JOYSTICK_X_MIRROR_L + d] = 0.0f;
        analogSignal[CAB1_JOYSTICK_Y_MIRROR_L + d] = 0.0f;
        analogSignal[CAB1_JOYSTICK_X_MIRROR_R + d] = 0.0f;
        analogSignal[CAB1_JOYSTICK_Y_MIRROR_R + d] = 0.0f;
        analogSignal[CAB1_JOYSTICK_Y_BLIND_L + d] = 0.0f;
        analogSignal[CAB1_JOYSTICK_Y_BLIND_R + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_BUFFERCOLOR_L + d] = TO_FLOAT(tumblers[TUMBLER_BUFFERCOLOR_L][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_BUFFERLIGHT_L + d] = TO_FLOAT(tumblers[TUMBLER_BUFFERLIGHT_L][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_BUFFERLIGHT_R + d] = TO_FLOAT(tumblers[TUMBLER_BUFFERLIGHT_R][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_BUFFERCOLOR_R + d] = TO_FLOAT(tumblers[TUMBLER_BUFFERCOLOR_R][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_CAB_HEATER1 + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_CAB_HEATER2 + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_MIRRORS_HEATER + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_WINDOWS_HEATER + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_HEATER_AUTO_OR_MANUAL + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_FLOOR_HEATER + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_WALL_HEATER + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_CAB_HEATERS + d] = 0.0f;

        // Панель слева
        analogSignal[CAB1_TUMBLER_SHASSIS_LIGHT + d] = TO_FLOAT(tumblers[TUMBLER_SHASSIS_LIGHT][cab_idx].getState());
        analogSignal[CAB1_TUMBLER_SPOTLIGHT_LOW + d] = TO_FLOAT(tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].getState());
        analogSignal[CAB1_TUMBLER_SPOTLIGHT_HIGH + d] = TO_FLOAT(tumblers[TUMBLER_SPOTLIGHT_HIGH][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_CAB_LIGHT_LOW + d] = TO_FLOAT(tumblers[TUMBLER_CAB_LIGHT_LOW][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_CAB_LIGHT_HIGH + d] = TO_FLOAT(tumblers[TUMBLER_CAB_LIGHT_HIGH][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_CAB_LIGHT_GREEN + d] = TO_FLOAT(tumblers[TUMBLER_CAB_LIGHT_GREEN][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_DEVICES_LIGHT + d] = TO_FLOAT(tumblers[TUMBLER_DEVICES_LIGHT][cab_idx].getState());
        analogSignal[CAB1_DIMMER_PANEL_BRIGHTNESS + d] = TO_FLOAT(switchers[SWITCHER_PANEL_BRIGHTNESS][cab_idx].getHandlePosition());
        analogSignal[CAB1_DIMMER_DEVICES_BRIGHTNESS + d] = TO_FLOAT(switchers[SWITCHER_DEVICES_BRIGHTNESS][cab_idx].getHandlePosition());
        analogSignal[CAB1_TOOGLE_WIPERS + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_WIPERS_FREQUENCY + d] = 0.0f;
        analogSignal[CAB1_BUTTON_RESERVOIR1 + d] = 0.0f;
        analogSignal[CAB1_BUTTON_RESERVOIR2 + d] = 0.0f;
        analogSignal[CAB1_BUTTON_RESERVOIR3 + d] = 0.0f;
        analogSignal[CAB1_BUTTON_WASHER + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_BATTERY_OR_EPB_VOLTAGE + d] = TO_FLOAT(tumblers[TUMBLER_BATTERY_OR_EPB_VOLTAGE][cab_idx].getState());
        analogSignal[CAB1_SIGLIGHT_EPB_CONTROL + d] = TO_FLOAT(epb_control->stateReleaseLamp());
        analogSignal[CAB1_SIGLIGHT_EPB_HOLD + d] = TO_FLOAT(epb_control->stateHoldLamp());
        analogSignal[CAB1_SIGLIGHT_EPB_BRAKE + d] = TO_FLOAT(epb_control->stateBrakeLamp());

        // Панель на столешнице слева
        analogSignal[CAB1_BUTTON_RELEASE_BRAKES + d] = TO_FLOAT(tumblers[BUTTON_RELEASE_BRAKES][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_EPK + d] = 1.0f;
        analogSignal[CAB1_TOOGLE_OIL_HEATING + d] = 0.0f;
        analogSignal[CAB1_BUTTON_COMPRESSOR + d] = TO_FLOAT(tumblers[BUTTON_COMPRESSOR][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_CONDITIONER + d] = 0.0f;
        analogSignal[CAB1_TOOGLE_SIGNAL_PANEL + d] = TO_FLOAT(tumblers[TUMBLER_SIGNAL_PANEL_BS_002][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_PCHF + d] = TO_FLOAT(tumblers[TUMBLER_PCHF][cab_idx].getState());
        analogSignal[CAB1_TOOGLE_USAVP + d] = TO_FLOAT(tumblers[TUMBLER_USAVP][cab_idx].getState());

        // Рукоятки бдительности
        analogSignal[CAB1_BUTTON_RB + d] = TO_FLOAT(tumblers[BUTTON_RB][cab_idx].getState());
        analogSignal[CAB1_BUTTON_RBS + d] = TO_FLOAT(tumblers[BUTTON_RBS][cab_idx].getState());
        analogSignal[CAB1_BUTTON_RBP + d] = TO_FLOAT(tumblers[BUTTON_RBP][cab_idx].getState());

        // Пульт помощника
        analogSignal[CAB1_VALVE_P_EMERGENCY_BRAKE + d] = TO_FLOAT(tumblers[BUTTON_EMERGENCY_BRAKE][cab_idx].getState());
        analogSignal[CAB1_BUTTON_P_MAIN_SWITCH_OFF + d] = TO_FLOAT(tumblers[BUTTON_MAIN_SWITCH_OFF][cab_idx].getState());
        analogSignal[CAB1_BUTTON_P_TYPHON + d] = TO_FLOAT(tumblers[BUTTON_P_TYPHON][cab_idx].getState());
        analogSignal[CAB1_BUTTON_P_WHISTLE + d] = TO_FLOAT(tumblers[BUTTON_P_WHISTLE][cab_idx].getState());

        // Табло сигнализации
        analogSignal[CAB1_SIGLIGHT_VIP_NO_IMPULSE + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_VIP_CIRCUIT_NO_READY + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_PRESSURE_BC_3 + d] = TO_FLOAT(signals_module->getLampState(SM_LAMP_TC3));
        analogSignal[CAB1_SIGLIGHT_PRESSURE_BC_2 + d] = TO_FLOAT(signals_module->getLampState(SM_LAMP_TC2));
        analogSignal[CAB1_SIGLIGHT_PRESSURE_BC_1 + d] = TO_FLOAT(signals_module->getLampState(SM_LAMP_TC1));
        analogSignal[CAB1_SIGLIGHT_TRANSFORMER_OIL_PUMP_OFF + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_LOW_VOLTAGE_PCHF + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_AUX_RELAY_SHORT_CIRCUIT + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_AUX_RELAY_NO_VOLTAGE + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_FAILURE_OIL_PRESSURE_MK_2 + d] = TO_FLOAT(signals_module->getLampState(SM_DM2));
        analogSignal[CAB1_SIGLIGHT_FAILURE_OIL_PRESSURE_MK_1 + d] = TO_FLOAT(signals_module->getLampState(SM_DM1));
        analogSignal[CAB1_SIGLIGHT_NO_BATTERY_CHARGE + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_FIRE_ALARM + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_RESERVE_1 + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_RESERVE_2 + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_POWER_RELAY_SHORT_CIRCUIT + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_MAIN_SWITCH_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_GV));
        analogSignal[CAB1_SIGLIGHT_RECTIFIER_FOR_FIELD_OFF + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_WORK_AT_LOW_FREQUENCY + d] = TO_FLOAT(msud->getOutputData().is_MV_low_freq);
        analogSignal[CAB1_SIGLIGHT_WEAK_FIELD + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_RESERVE_3 + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_WHEELSLIP + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_FAN_4_OFF + d] = 0.0f;
        analogSignal[CAB1_SIGLIGHT_FAN_3_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_V3));
        analogSignal[CAB1_SIGLIGHT_FAN_2_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_V2));
        analogSignal[CAB1_SIGLIGHT_FAN_1_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_V1));
        analogSignal[CAB1_SIGLIGHT_COMPRESSOR_2_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_MK2));
        analogSignal[CAB1_SIGLIGHT_COMPRESSOR_1_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_MK1));
        analogSignal[CAB1_SIGLIGHT_FAST_SWITCH_ENGINE_6_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_TD6));
        analogSignal[CAB1_SIGLIGHT_FAST_SWITCH_ENGINE_5_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_TD5));
        analogSignal[CAB1_SIGLIGHT_FAST_SWITCH_ENGINE_4_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_TD4));
        analogSignal[CAB1_SIGLIGHT_FAST_SWITCH_ENGINE_3_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_TD3));
        analogSignal[CAB1_SIGLIGHT_FAST_SWITCH_ENGINE_2_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_TD2));
        analogSignal[CAB1_SIGLIGHT_FAST_SWITCH_ENGINE_1_OFF + d] = TO_FLOAT(signals_module->getLampState(SM_TD1));
    }


    analogSignal[KLUB_U_CAB1_POWER] = TO_FLOAT(Ucc >= 49);
    analogSignal[KLUB_U_CAB2_POWER] = TO_FLOAT(Ucc >= 49);

    QString text = klub_BEL->getStationText();
    for (size_t i = 0; i < text.size(); ++i)
    {
        analogSignal[KLUB_U_STATION_SYMB1 + i] = TO_FLOAT(text[i].unicode());
    }

    text = klub_BEL->getInfoText();
    for (size_t i = 0; i < text.size(); ++i)
    {
        analogSignal[KLUB_U_STRING_SYMB1 + i] = TO_FLOAT(text[i].unicode());
    }

    analogSignal[KLUB_U_SHEDULE_TIME] = 0.0f;

    analogSignal[KLUB_U_COORDINATE] = TO_FLOAT(klub_BEL->getRailCoord());

    analogSignal[KLUB_U_ZAPRET_OTPUSKA] = 0.0f;
    analogSignal[KLUB_U_EPK] = static_cast<float>(epk[CAB1]->isKeyOn()) +
                        2.0f * static_cast<float>(epk[CAB2]->isKeyOn());
    analogSignal[KLUB_U_SPEED] = TO_FLOAT(klub_BEL->getVelocityKmh());
    analogSignal[KLUB_U_SPEED_LIMIT] = TO_FLOAT(klub_BEL->getCurrentSpeedLimit());
    analogSignal[KLUB_U_SPEED_LIMIT_2] = TO_FLOAT(klub_BEL->getNextSpeedLimit());
    analogSignal[KLUB_U_ALSN] = TO_FLOAT(klub_BEL->getLampNum());
    analogSignal[KLUB_U_ALSN_FB] = 1.0f;
    analogSignal[KLUB_U_STRAIGHT] = 0.0f;
    analogSignal[KLUB_U_SIDE] = 0.0f;
    analogSignal[KLUB_U_BDITELNOST] = TO_FLOAT(klub_BEL->isCheckVigilanse());
    analogSignal[KLUB_U_M] = static_cast<float>(klub_BEL->isShuntingMode());
    analogSignal[KLUB_U_P] = static_cast<float>(!klub_BEL->isShuntingMode());
    analogSignal[KLUB_U_CASSETE] = 1.0f;
    analogSignal[KLUB_U_REVERSOR] = TO_FLOAT(epk[CAB1]->isKeyOn() * km[CAB1]->getReversHandlePos() +
                                             epk[CAB2]->isKeyOn() * km[CAB2]->getReversHandlePos());
    analogSignal[KLUB_U_TARGET_DIST] = TO_FLOAT(klub_BEL->getTargetDistance());
    analogSignal[KLUB_U_PRESSURE_TM] = TO_FLOAT(brakepipe->getPressure());
    analogSignal[KLUB_U_PRESSURE_UR1] = TO_FLOAT(brake_crane[CAB1]->getERpressure());
    analogSignal[KLUB_U_PRESSURE_UR2] = TO_FLOAT(brake_crane[CAB2]->getERpressure());
    analogSignal[KLUB_U_TRACK_NUM] = 1.0f;
    analogSignal[KLUB_U_ACCELERATION] = TO_FLOAT(klub_BEL->getAcceleration());

    bool is_visible = (msud->getOutputData().state == MSUD_READY);
    if (is_visible)
    {
        auto cab_idx = CAB1;
        if (tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_MSUD))
        {
            analogSignal[MSUD_CAB1_POWER] = 1.0f;
        }
        else
        {
            analogSignal[MSUD_CAB1_POWER] = 0.0f;

            if (tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_MSUD))
            {
                analogSignal[MSUD_CAB2_POWER] = 1.0f;
                cab_idx = CAB2;
            }
            else
            {
                analogSignal[MSUD_CAB2_POWER] = 0.0f;
            }
        }

        analogSignal[MSUD_SPEED2] = TO_FLOAT(km[cab_idx]->getRefSpeedLevel() * msud->getOutputData().Vmax);
        analogSignal[MSUD_SPEED1] = TO_FLOAT(velocity * Physics::kmh);

        double Ia_max = 0;
        double trac_level = 0;

        if (msud_input.is_traction)
        {
            Ia_max = msud->getOutputData().Ia_max;
            trac_level = qAbs(msud_input.Ia[TRAC_MOTOR1] * 100.0 / Ia_max);
        }

        if (msud_input.is_brake)
        {
            Ia_max = msud->getOutputData().Ib_max;
            trac_level = qAbs(msud_input.Ia[TRAC_MOTOR1] * 100.0 / Ia_max);
        }

        analogSignal[MSUD_CURRENT_ANHCOR2] = TO_FLOAT( (km[cab_idx]->getTracLevel() +
                                                       std::abs(km[cab_idx]->getBrakeLevel())) * Ia_max);

        analogSignal[MSUD_CURRENT_ANHCOR1] = TO_FLOAT(std::abs(msud_input.Ia[TRAC_MOTOR1]));

        trac_level = cut(trac_level, 0.0, 100.0);
        analogSignal[MSUD_TRACTION] = TO_FLOAT(trac_level);

        analogSignal[MSUD_MK] = TO_FLOAT(!motor_compressor->isPowered() && press_reg->getState());
        analogSignal[MSUD_DM] = TO_FLOAT(!motor_compressor->isPowered());
        analogSignal[MSUD_DB] = 0.0f;
        analogSignal[MSUD_KZ] = 0.0f;
        analogSignal[MSUD_OV] = 0.0f;

        analogSignal[MSUD_CURCUIT_VOZB] = TO_FLOAT(trac_motor[TRAC_MOTOR1]->getFieldCurrent());

        analogSignal[MSUD_CURRENT_EPT] = TO_FLOAT(std::abs(epb_converter->getOutputCurrent()));
        analogSignal[MSUD_VOLTAGE_EPT] = TO_FLOAT(epb_converter->getOutputVoltage());

        analogSignal[MSUD_NC] = TO_FLOAT(msud->getOutputData().is_MV_low_freq);
        analogSignal[MSUD_MPK] = TO_FLOAT(static_cast<int>(msud_input.tumbler_MPK) + 1);
        analogSignal[MSUD_MODE] = TO_FLOAT(static_cast<int>(msud_input.is_auto_reg) + 1);

        bool is_MSUD_OB = main_switch->getU_out() >= 10000 && battery->getChargeCurrent() <= 0.0;
        analogSignal[MSUD_OB] = TO_FLOAT(is_MSUD_OB);

        analogSignal[MSUD_TC] = TO_FLOAT(msud->getOutputData().TC_status);

        analogSignal[MSUD_VIP_ZONE] = TO_FLOAT(msud->getOutputData().vip_voltage_level);

        if (reversor->getState() == 1)
            analogSignal[MSUD_REVERSOR] = 1.0f;

        if (reversor->getState() == -1)
            analogSignal[MSUD_REVERSOR] = 2.0f;

        if (qt1->getContactState(9))
            analogSignal[MSUD_TRACTION_TYPE] = 1.0f;
        else
            analogSignal[MSUD_TRACTION_TYPE] = 2.0f;


        if (msud_input.is_traction || msud_input.is_brake)
            analogSignal[MSUD_TRACTION_STATE] = 1.0f;
        else
            analogSignal[MSUD_TRACTION_STATE] = 2.0f;

        bool is_DM_low = signals_module->getLampState(SM_DM1) ||
                         signals_module->getLampState(SM_DM2);

        if (is_DM_low)
            analogSignal[MSUD_DM] = 1.0f;
        else
            analogSignal[MSUD_DM] = 0.0f;

        analogSignal[MSUD_OSLAB_POLE1] = TO_FLOAT(msud->getOutputData().op[STEP1]);
        analogSignal[MSUD_OSLAB_POLE2] = TO_FLOAT(msud->getOutputData().op[STEP2]);
        analogSignal[MSUD_OSLAB_POLE3] = TO_FLOAT(msud->getOutputData().op[STEP3]);
    }
    else
    {
        analogSignal[MSUD_CAB1_POWER] = 0.0f;
        analogSignal[MSUD_CAB2_POWER] = 0.0f;
    }
}
