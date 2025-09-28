#include    "vl60k.h"

#include "vl60-signals.h"

#include "alsn-ukbm.h"
#include "brake-crane.h"
#include "dc-motor.h"
#include "ekg-8g.h"
#include "kme-60-044.h"
#include "loco-crane.h"
#include "motor-fan-ac.h"
#include "oscillator.h"
#include "pantograph.h"
#include "phase-splitter.h"
#include "pneumo-brake-lock.h"
#include "pneumo-splitter.h"
#include "protective-device.h"
#include "reservoir.h"
#include "sl2m.h"
#include "sanding-system.h"
#include "train-horn.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::signalsOutput(const simulator_time_t& t, const double& dt)
{
    (void) t;
    (void) dt;

    analogSignal[SERIAL_NUMBER] = 1737.0f;

    // Вращение колёсных пар
    analogSignal[WHEELSET_1] = static_cast<float>(wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_2] = static_cast<float>(wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_3] = static_cast<float>(wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_4] = static_cast<float>(wheel_rotation_angle[3] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_5] = static_cast<float>(wheel_rotation_angle[4] / 2.0 / Physics::PI);
    analogSignal[WHEELSET_6] = static_cast<float>(wheel_rotation_angle[5] / 2.0 / Physics::PI);

    // Состояние главного выключателя
    analogSignal[MAIN_SWITCH_POS] = static_cast<float>(main_switch->getKnifePos());

    // Состояние токоприемников
    analogSignal[PANT1_POS] = static_cast<float>(pantographs[0]->getHeight());
    analogSignal[PANT2_POS] = static_cast<float>(pantographs[1]->getHeight());

    // Кабины
    for (auto cab_idx : {CAB1, CAB2})
    {
        std::uint16_t d = (SPOTLIGHT_BWD - SPOTLIGHT_FWD) * cab_idx;

        // Освещение
        bool is_power = (U_bat > 0.9 * 50.0);
        if (is_power)
        {
            // Прожектор
            if (spotlight_low_tumbler[cab_idx].getState())
            {
                std::uint8_t intensity = 1 + spotlight_high_tumbler[cab_idx].getState();
                analogSignal[SPOTLIGHT_FWD + d] = static_cast<float>(intensity) / 2.0f;
            }
            else
            {
                analogSignal[SPOTLIGHT_FWD + d] = 0.0f;
            }

            // Левый буферный
            if (P_bufferlight_L_tumbler[cab_idx].getState())
            {
                analogSignal[BUFFERLIGHT_FWD_L_WHITE + d] = static_cast<float>(P_buffercolor_L_toogle[cab_idx].getState());
                analogSignal[BUFFERLIGHT_FWD_L_RED + d] = static_cast<float>(!P_buffercolor_L_toogle[cab_idx].getState());
            }
            else
            {
                analogSignal[BUFFERLIGHT_FWD_L_WHITE + d] = 0.0f;
                analogSignal[BUFFERLIGHT_FWD_L_RED + d] = 0.0f;
            }

            // Правый буферный
            if (P_bufferlight_R_tumbler[cab_idx].getState())
            {
                analogSignal[BUFFERLIGHT_FWD_R_WHITE + d] = static_cast<float>(P_buffercolor_R_toogle[cab_idx].getState());
                analogSignal[BUFFERLIGHT_FWD_R_RED + d] = static_cast<float>(!P_buffercolor_R_toogle[cab_idx].getState());
            }
            else
            {
                analogSignal[BUFFERLIGHT_FWD_R_WHITE + d] = 0.0f;
                analogSignal[BUFFERLIGHT_FWD_R_RED + d] = 0.0f;
            }

            // Освещение кабины
            if (P_cab_light_low_tumbler[cab_idx].getState())
            {
                std::uint8_t intensity = 1 + P_cab_light_high_tumbler[cab_idx].getState();
                analogSignal[CAB1_LIGHT_CABINE + d] = static_cast<float>(intensity) / 2.0f;
            }
            else
            {
                analogSignal[CAB1_LIGHT_CABINE + d] = 0.0f;
            }

            // Подсветка приборов
            analogSignal[CAB1_LIGHT_DEVICES + d] = static_cast<float>(P_light_devices_tumbler[cab_idx].getState());

            // Локомотивный светофор
            analogSignal[CAB1_LS_WHITE + d] = safety_device[cab_idx]->getWhiteLamp();
            analogSignal[CAB1_LS_RED + d] = safety_device[cab_idx]->getRedLamp();
            analogSignal[CAB1_LS_REDYELLOW + d] = safety_device[cab_idx]->getRedYellowLamp();
            analogSignal[CAB1_LS_YELLOW + d] = safety_device[cab_idx]->getYellowLamp();
            analogSignal[CAB1_LS_GREEN + d] = safety_device[cab_idx]->getGreenLamp();
            // Приборы безопасности
            analogSignal[CAB1_SIGLIGHT_PSS + d] = safety_device[cab_idx]->getStatePSS();
            analogSignal[CAB1_SIGLIGHT_PROPUSK + d] = 0.0f;

            // Сигнальные лампы
            analogSignal[CAB1_SIGLIGHT_TR + d] = static_cast<float>(!motor_fans[MV3]->isReady() || !motor_fans[MV4]->isReady());
            analogSignal[CAB1_SIGLIGHT_TD + d] = isLineContactorsOff();
            analogSignal[CAB1_SIGLIGHT_0HP + d] = static_cast<float>(main_controller->isLongMotionPos());
            analogSignal[CAB1_SIGLIGHT_VU1 + d] = static_cast<float>(!motor_fans[MV1]->isReady() || !motor_fans[MV2]->isReady());
            analogSignal[CAB1_SIGLIGHT_VU2 + d] = static_cast<float>(!motor_fans[MV5]->isReady() || !motor_fans[MV6]->isReady());

            analogSignal[CAB1_SIGLIGHT_TR_SME + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_TD_SME + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_0HP_SME + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_VU1_SME + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_VU2_SME + d] = 0.0f;

            analogSignal[CAB1_SIGLIGHT_FR + d] = phase_spliter->isNotReady();
            analogSignal[CAB1_SIGLIGHT_GU + d] = phase_spliter->isNotReady();
            analogSignal[CAB1_SIGLIGHT_RB + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_GV + d] = main_switch->getLampState();
            analogSignal[CAB1_SIGLIGHT_RZ_RKZ + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_RP + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_RPO + d] = 0.0f;
        }
        else
        {
            analogSignal[SPOTLIGHT_FWD + d] = 0.0f;
            analogSignal[BUFFERLIGHT_FWD_L_WHITE + d] = 0.0f;
            analogSignal[BUFFERLIGHT_FWD_L_RED + d] = 0.0f;
            analogSignal[BUFFERLIGHT_FWD_R_WHITE + d] = 0.0f;
            analogSignal[BUFFERLIGHT_FWD_R_RED + d] = 0.0f;
            analogSignal[CAB1_LIGHT_CABINE + d] = 0.0f;
            analogSignal[CAB1_LIGHT_DEVICES + d] = 0.0f;

            analogSignal[CAB1_LS_WHITE + d] = 0.0f;
            analogSignal[CAB1_LS_RED + d] = 0.0f;
            analogSignal[CAB1_LS_REDYELLOW + d] = 0.0f;
            analogSignal[CAB1_LS_YELLOW + d] = 0.0f;
            analogSignal[CAB1_LS_GREEN + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_PSS + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_PROPUSK + d] = 0.0f;

            analogSignal[CAB1_SIGLIGHT_TR + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_TD + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_0HP + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_VU1 + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_VU2 + d] = 0.0f;

            analogSignal[CAB1_SIGLIGHT_EPB_CONTROL + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_EPB_HOLD + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_EPB_BRAKE + d] = 0.0f;
            analogSignal[CAB1_EPB_AMPERMETER + d] = 0.0f;

            analogSignal[CAB1_SIGLIGHT_FR + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_GU + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_RB + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_GV + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_RZ_RKZ + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_RP + d] = 0.0f;
            analogSignal[CAB1_SIGLIGHT_RPO + d] = 0.0f;
        }

        // Скоростемер
        analogSignal[CAB1_3SL2M_SPEED + d] = speed_meter[cab_idx]->getArrowPos();
        analogSignal[CAB1_3SL2M_SHAFT + d] = speed_meter[cab_idx]->getShaftPos();

        // Циферблаты
        analogSignal[CAB1_P_AUX_PRESSURE + d] = 0.0f;
        analogSignal[CAB1_P_BATTERY_VOLTMETER + d] = static_cast<float>(U_bat / 150.0);
        analogSignal[CAB1_INPUT_VOLTAGE + d] = static_cast<float>(main_switch->getU_out() / 30000.0);
        analogSignal[CAB1_ENGINE_VOLTAGE + d] = static_cast<float>(gauge_KV_motors->getOutput() / 3000.0);
        if (cab_idx == CAB1)
        {
            analogSignal[CAB1_ENGINE_CURRENT_FWD] = static_cast<float>(motor[TED1]->getIa() / 1500.0);
            analogSignal[CAB1_ENGINE_CURRENT_BWD] = static_cast<float>(motor[TED4]->getIa() / 1500.0);
        }
        else
        {
            analogSignal[CAB1_ENGINE_CURRENT_FWD + d] = static_cast<float>(motor[TED6]->getIa() / 1500.0);
            analogSignal[CAB1_ENGINE_CURRENT_BWD + d] = static_cast<float>(motor[TED3]->getIa() / 1500.0);
        }
        analogSignal[CAB1_SELSIN_EKG_POS + d] = main_controller->getSelsinPosition();
        analogSignal[CAB1_PRESSURE_BC + d] = static_cast<float>(bc_splitter->getInputPressure() / 1.0);
        analogSignal[CAB1_PRESSURE_FL + d] = static_cast<float>(main_reservoir->getPressure() / 1.6);
        analogSignal[CAB1_PRESSURE_BP + d] = static_cast<float>(brakepipe->getPressure() / 1.0);
        analogSignal[CAB1_PRESSURE_ER + d] = static_cast<float>(brake_crane[cab_idx]->getERpressure() / 1.0);

        // Контроллер машиниста
        analogSignal[CAB1_KM_IS_REVERS_HANDLE + d] = static_cast<float>(controller[cab_idx]->isReversHandle());
        analogSignal[CAB1_KM_REVERS_HANDLE_POS + d] = controller[cab_idx]->getReversHandlePos();
        analogSignal[CAB1_KM_MAIN_HANDLE_POS + d] = controller[cab_idx]->getMainHandlePos();

        // Рукоятка УБТ, комбинированный кран, поездной кран, локомотивный кран
        analogSignal[CAB1_UBT_IS_KEY_HANDLE + d] = static_cast<float>(brake_lock[cab_idx]->isLockHandle());
        analogSignal[CAB1_UBT_KEY_HANDLE_POS + d] = static_cast<float>(brake_lock[cab_idx]->getLockHandlePosition());
        analogSignal[CAB1_UBT_COMBINE_CRANE_POS + d] = static_cast<float>(brake_lock[cab_idx]->getCombineCraneHandlePosition());
        analogSignal[CAB1_BRAKE_CRANE_HANDLE_POS + d] = static_cast<float>(brake_crane[cab_idx]->getHandlePosition());
        analogSignal[CAB1_LOCO_CRANE_HANDLE_POS + d] = static_cast<float>(loco_crane[cab_idx]->getHandlePosition());

        // ЭПК
        analogSignal[CAB1_AUTOSTOP_SHUTOFF_POS + d] = 1.0f;
        analogSignal[CAB1_AUTOSTOP_KEY_POS + d] = static_cast<float>(key_epk[cab_idx].getState());

        // Дальний ряд тумблеров пульта машиниста
        analogSignal[CAB1_TUMBLER_SPOTLIGHT_HIGH + d] = static_cast<float>(spotlight_high_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_SPOTLIGHT_LOW + d] = static_cast<float>(spotlight_low_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_RADIO + d] = static_cast<float>(radio_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_CONTROL_LINES + d] = static_cast<float>(cu_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_PANT1 + d] = static_cast<float>(pant1_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_PANT2 + d] = static_cast<float>(pant2_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_PANTS + d] = static_cast<float>(pants_tumbler[cab_idx].getState());
        analogSignal[CAB1_RETURN_MAIN_SWITCH + d] = static_cast<float>(gv_return_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_MAIN_SWITCH + d] = static_cast<float>(gv_tumbler[cab_idx].getState());

        // Ближний ряд тумблеров пульта машиниста
        analogSignal[CAB1_TUMBLER_AUTOSAND + d] = static_cast<float>(autosand_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_FAN6 + d] = static_cast<float>(mv_tumblers[cab_idx][MV6].getState());
        analogSignal[CAB1_TUMBLER_FAN5 + d] = static_cast<float>(mv_tumblers[cab_idx][MV5].getState());
        analogSignal[CAB1_TUMBLER_FAN4 + d] = static_cast<float>(mv_tumblers[cab_idx][MV4].getState());
        analogSignal[CAB1_TUMBLER_FAN3 + d] = static_cast<float>(mv_tumblers[cab_idx][MV3].getState());
        analogSignal[CAB1_TUMBLER_FAN2 + d] = static_cast<float>(mv_tumblers[cab_idx][MV2].getState());
        analogSignal[CAB1_TUMBLER_FAN1 + d] = static_cast<float>(mv_tumblers[cab_idx][MV1].getState());
        analogSignal[CAB1_TUMBLER_COMPRESSOR + d] = static_cast<float>(mk_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_PHASESPLITTER + d] = static_cast<float>(fr_tumbler[cab_idx].getState());

        // Ряд тумблеров на приборной панели помощника машиниста
        analogSignal[CAB1_TUMBLER_P_TIFON + d] = static_cast<float>(P_tifon_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_WHISTLE + d] = static_cast<float>(P_whistle_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_CAB_HEAT + d] = static_cast<float>(P_cab_heat_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_CAB_LIGHT_LOW + d] = static_cast<float>(P_cab_light_low_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_CAB_LIGHT_HIGH + d] = static_cast<float>(P_cab_light_high_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_RESERVE1 + d] = static_cast<float>(P_reserv1_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_SHASSIS_LIGHT + d] = static_cast<float>(P_light_chassis_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_DEVICES_LIGHT + d] = static_cast<float>(P_light_devices_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_BUFFERLIGHT_L + d] = static_cast<float>(P_bufferlight_L_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_BUFFERLIGHT_R + d] = static_cast<float>(P_bufferlight_R_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_RESERVE2 + d] = static_cast<float>(P_reserv2_tumbler[cab_idx].getState());
        analogSignal[CAB1_TUMBLER_P_ALSN_CHECK + d] = static_cast<float>(P_ALSN_check_tumbler[cab_idx].getState());
        analogSignal[CAB1_TOOGLE_P_BUFFERCOLOR_L + d] = static_cast<float>(P_buffercolor_L_toogle[cab_idx].getState());
        analogSignal[CAB1_TOOGLE_P_BUFFERCOLOR_R + d] = static_cast<float>(P_buffercolor_R_toogle[cab_idx].getState());

        analogSignal[CAB1_RBS + d] = static_cast<float>(rb[cab_idx][RBS].getState());
        analogSignal[CAB1_RB_1 + d] = static_cast<float>(rb[cab_idx][RB_1].getState());
        analogSignal[CAB1_RBP + d] = static_cast<float>(rb[cab_idx][RBP].getState());
        analogSignal[CAB1_TIFON + d] = static_cast<float>(horn[cab_idx]->isTifon());
        analogSignal[CAB1_WHISTLE + d] = static_cast<float>(horn[cab_idx]->isSvistok());
        analogSignal[CAB1_SAND + d] = static_cast<float>(sand_system->isSandDelivery());
    }
}
