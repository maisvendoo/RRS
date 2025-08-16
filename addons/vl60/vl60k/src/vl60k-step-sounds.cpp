#include    "vl60k.h"

#include "vl60-signals.h"

#include "automatic-train-stop.h"
#include "brake-crane.h"
#include "brake-lock.h"
#include "dc-motor.h"
#include "ekg-8g.h"
#include "kme-60-044.h"
#include "loco-crane.h"
#include "motor-compressor-ac.h"
#include "motor-fan-ac.h"
#include "pantograph.h"
#include "phase-splitter.h"
#include "protective-device.h"
#include "sanding-system.h"
#include "sl2m.h"
#include "trac-transformer.h"
#include "train-horn.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::stepSoundSignalsOutput(double t, double dt)
{
    (void) t;
    (void) dt;

    // Озвучка кабин
    for (auto cab_idx : {CAB1, CAB2})
    {
        std::uint16_t d = (SOUND_BWD_SVISTOK - SOUND_FWD_SVISTOK) * cab_idx;

        // Свисток и тифон
        analogSignal[SOUND_FWD_SVISTOK + d] = horn[cab_idx]->getSoundSignal(TrainHorn::SVISTOK_SOUND);
        analogSignal[SOUND_FWD_TIFON + d] = horn[cab_idx]->getSoundSignal(TrainHorn::TIFON_SOUND);

        // Реверсор и контроллер
        analogSignal[CAB1_SOUND_INSERT_REVERS_HANDLE + d] = controller[cab_idx]->getSoundSignal(ControllerKME_60_044::HANDLE_INSERTED_SOUND);
        analogSignal[CAB1_SOUND_REMOVE_REVERS_HANDLE + d] = controller[cab_idx]->getSoundSignal(ControllerKME_60_044::HANDLE_REMOVED_SOUND);
        analogSignal[CAB1_SOUND_CHANGE_REVERS_POS + d] = controller[cab_idx]->getSoundSignal(ControllerKME_60_044::REVERS_CHANGE_POS_SOUND);
        analogSignal[CAB1_SOUND_CHANGE_MAIN_POS + d] = controller[cab_idx]->getSoundSignal(ControllerKME_60_044::MAIN_CHANGE_POS_SOUND);

        // Устройство блокировки тормозов
        analogSignal[CAB1_SOUND_INSERT_BRAKE_LOCK_HANDLE + d] = -1.0f /*brake_lock[cab_idx]->getSoundSignal(BrakeLock::CHANGE_LOCK_POS_SOUND)*/;
        analogSignal[CAB1_SOUND_REMOVE_BRAKE_LOCK_HANDLE + d] = -1.0f /*brake_lock[cab_idx]->getSoundSignal(BrakeLock::CHANGE_LOCK_POS_SOUND)*/;
        analogSignal[CAB1_SOUND_BRAKE_LOCK_CHANGE_LOCK_POS + d] = brake_lock[cab_idx]->getSoundSignal(BrakeLock::CHANGE_LOCK_POS_SOUND);
        analogSignal[CAB1_SOUND_BRAKE_LOCK_CHANGE_COMB_POS + d] = brake_lock[cab_idx]->getSoundSignal(BrakeLock::CHANGE_COMB_POS_SOUND);
        analogSignal[CAB1_SOUND_BRAKE_LOCK_BP_DRAIN_FLOW + d] = brake_lock[cab_idx]->getSoundSignal(BrakeLock::BP_DRAIN_FLOW_SOUND);

        // Поездной кран
        analogSignal[CAB1_SOUND_BRAKE_CRANE_CHANGE_POS + d] = brake_crane[cab_idx]->getSoundSignal(BrakeCrane::CHANGE_POS_SOUND);
        analogSignal[CAB1_SOUND_BRAKE_CRANE_ER_STAB_FLOW + d] = brake_crane[cab_idx]->getSoundSignal(BrakeCrane::ER_STAB_SOUND);
        analogSignal[CAB1_SOUND_BRAKE_CRANE_ER_FILL_FLOW + d] = brake_crane[cab_idx]->getSoundSignal(BrakeCrane::ER_FILL_FLOW_SOUND);
        analogSignal[CAB1_SOUND_BRAKE_CRANE_ER_DRAIN_FLOW + d] = brake_crane[cab_idx]->getSoundSignal(BrakeCrane::ER_DRAIN_FLOW_SOUND);
        analogSignal[CAB1_SOUND_BRAKE_CRANE_BP_FILL_FLOW + d] = brake_crane[cab_idx]->getSoundSignal(BrakeCrane::BP_FILL_FLOW_SOUND);
        analogSignal[CAB1_SOUND_BRAKE_CRANE_BP_DRAIN_FLOW + d] = brake_crane[cab_idx]->getSoundSignal(BrakeCrane::BP_DRAIN_FLOW_SOUND);

        // Локомотивный кран
        analogSignal[CAB1_SOUND_LOCO_CRANE_CHANGE_POS + d] = loco_crane[cab_idx]->getSoundSignal(LocoCrane::CHANGE_POS_SOUND);
        analogSignal[CAB1_SOUND_LOCO_CRANE_BC_FILL_FLOW + d] = loco_crane[cab_idx]->getSoundSignal(LocoCrane::BC_FILL_FLOW_SOUND);
        analogSignal[CAB1_SOUND_LOCO_CRANE_BC_DRAIN_FLOW + d] = loco_crane[cab_idx]->getSoundSignal(LocoCrane::BC_DRAIN_FLOW_SOUND);

        // ЭПК
        analogSignal[CAB1_SOUND_AUTOSTOP_WHISTLE + d] = epk[cab_idx]->getSoundSignal();

        // Дальний ряд тумблеров приборной панели машиниста
        analogSignal[CAB1_SOUND_TUMBLER_PROJECTOR2_ON + d] = spotlight_high_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PROJECTOR1_ON + d] = spotlight_low_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_RADIO_ON + d] = radio_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_CTRL_CIRCUIT_ON + d] = cu_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PANT_BWD_ON + d] = pant2_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PANT_FWD_ON + d] = pant1_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PANTS_ON + d] = pants_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_RETURN_MAIN_SWITCH_ON + d] = gv_return_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_MAIN_SWITCH_ON + d] = gv_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);

        // Ближний ряд тумблеров приборной панели машиниста
        analogSignal[CAB1_SOUND_TUMBLER_AUTOSAND_ON + d] = autosand_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN6_ON + d] = mv_tumblers[cab_idx][MV6].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN5_ON + d] = mv_tumblers[cab_idx][MV5].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN4_ON + d] = mv_tumblers[cab_idx][MV4].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN3_ON + d] = mv_tumblers[cab_idx][MV3].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN2_ON + d] = mv_tumblers[cab_idx][MV2].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN1_ON + d] = mv_tumblers[cab_idx][MV1].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_COMPRESSOR_ON + d] = mk_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PHASESPLITTER_ON + d] = fr_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);

        // Ряд тумблеров на приборной панели помощника машиниста
        analogSignal[CAB1_SOUND_TUMBLER_P_TIFON_ON + d] = P_tifon_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_WHISTLE_ON + d] = P_whistle_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_CAB_HEAT_ON + d] = P_cab_heat_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_CAB_LIGHT_LOW_ON + d] = P_cab_light_low_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_CAB_LIGHT_HIGH_ON + d] = P_cab_light_high_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_RESERVE1_ON + d] = P_reserv1_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_SHASSIS_LIGHT_ON + d] = P_light_chassis_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_DEVICES_LIGHT_ON + d] = P_light_devices_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_BUFFERLIGHT_L_ON + d] = P_bufferlight_L_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_BUFFERLIGHT_R_ON + d] = P_bufferlight_R_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_RESERVE2_ON + d] = P_reserv2_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_ALSN_CHECK_ON + d] = P_ALSN_check_tumbler[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TOOGLE_P_BUFFERCOLOR_L_UP + d] = P_buffercolor_L_toogle[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_TOOGLE_P_BUFFERCOLOR_R_UP + d] = P_buffercolor_R_toogle[cab_idx].getSoundSignal(Trigger::ON_SOUND);
        analogSignal[CAB1_SOUND_EPK_ON + d] = key_epk[cab_idx].getSoundSignal(Trigger::ON_SOUND);

        // Скоростемер
        analogSignal[CAB1_SOUND_SPEED_METER_SL2M + d] = speed_meter[cab_idx]->getSoundSignal();

        // Дальний ряд тумблеров приборной панели машиниста
        analogSignal[CAB1_SOUND_TUMBLER_PROJECTOR2_OFF + d] = spotlight_high_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PROJECTOR1_OFF + d] = spotlight_low_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_RADIO_OFF + d] = radio_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_CTRL_CIRCUIT_OFF + d] = cu_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PANT_BWD_OFF + d] = pant2_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PANT_FWD_OFF + d] = pant1_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PANTS_OFF + d] = pants_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_RETURN_MAIN_SWITCH_OFF + d] = gv_return_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_MAIN_SWITCH_OFF + d] = gv_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);

        // Ближний ряд тумблеров приборной панели машиниста
        analogSignal[CAB1_SOUND_TUMBLER_AUTOSAND_OFF + d] = autosand_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN6_OFF + d] = mv_tumblers[cab_idx][MV6].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN5_OFF + d] = mv_tumblers[cab_idx][MV5].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN4_OFF + d] = mv_tumblers[cab_idx][MV4].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN3_OFF + d] = mv_tumblers[cab_idx][MV3].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN2_OFF + d] = mv_tumblers[cab_idx][MV2].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_FAN1_OFF + d] = mv_tumblers[cab_idx][MV1].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_COMPRESSOR_OFF + d] = mk_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_PHASESPLITTER_OFF + d] = fr_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);

        // Ряд тумблеров на приборной панели помощника машиниста
        analogSignal[CAB1_SOUND_TUMBLER_P_TIFON_OFF + d] = P_tifon_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_WHISTLE_OFF + d] = P_whistle_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_CAB_HEAT_OFF + d] = P_cab_heat_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_CAB_LIGHT_LOW_OFF + d] = P_cab_light_low_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_CAB_LIGHT_HIGH_OFF + d] = P_cab_light_high_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_RESERVE1_OFF + d] = P_reserv1_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_SHASSIS_LIGHT_OFF + d] = P_light_chassis_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_DEVICES_LIGHT_OFF + d] = P_light_devices_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_BUFFERLIGHT_L_OFF + d] = P_bufferlight_L_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_BUFFERLIGHT_R_OFF + d] = P_bufferlight_R_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_RESERVE2_OFF + d] = P_reserv2_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TUMBLER_P_ALSN_CHECK_OFF + d] = P_ALSN_check_tumbler[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TOOGLE_P_BUFFERCOLOR_L_DOWN + d] = P_buffercolor_L_toogle[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_TOOGLE_P_BUFFERCOLOR_R_DOWN + d] = P_buffercolor_R_toogle[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
        analogSignal[CAB1_SOUND_EPK_OFF + d] = key_epk[cab_idx].getSoundSignal(Trigger::OFF_SOUND);
    }

    // Звуки в движении
    double Vkmh = abs(velocity) * Physics::kmh;
    analogSignal[SOUND_5_10] = sound_state_t::createSoundSignal((Vkmh > 1.0) && (Vkmh <= 10.0), Vkmh / 5.0);
    analogSignal[SOUND_10_20] = sound_state_t::createSoundSignal((Vkmh > 10.0) && (Vkmh <= 20.0));
    analogSignal[SOUND_20_30] = sound_state_t::createSoundSignal((Vkmh > 20.0) && (Vkmh <= 30.0));
    analogSignal[SOUND_30_40] = sound_state_t::createSoundSignal((Vkmh > 30.0) && (Vkmh <= 40.0));
    analogSignal[SOUND_40_50] = sound_state_t::createSoundSignal((Vkmh > 40.0) && (Vkmh <= 50.0));
    analogSignal[SOUND_50_60] = sound_state_t::createSoundSignal((Vkmh > 50.0) && (Vkmh <= 60.0));
    analogSignal[SOUND_60_70] = sound_state_t::createSoundSignal((Vkmh > 60.0) && (Vkmh <= 70.0));
    analogSignal[SOUND_70_80] = sound_state_t::createSoundSignal((Vkmh > 70.0) && (Vkmh <= 80.0));
    analogSignal[SOUND_80_90] = sound_state_t::createSoundSignal((Vkmh > 80.0) && (Vkmh <= 90.0));
    analogSignal[SOUND_90_100] = sound_state_t::createSoundSignal((Vkmh > 90.0) && (Vkmh <= 100.0));
    analogSignal[SOUND_100_110] = sound_state_t::createSoundSignal((Vkmh > 100.0) && (Vkmh <= 110.0));
    analogSignal[SOUND_110_X] = sound_state_t::createSoundSignal(Vkmh > 110.0);

    // Токоприёмники
    analogSignal[SOUND_PANT_BWD_UP] = pantographs[1]->getSoundSignal(Pantograph::UP_SOUND);
    analogSignal[SOUND_PANT_BWD_DOWN] = pantographs[1]->getSoundSignal(Pantograph::DOWN_SOUND);
    analogSignal[SOUND_PANT_FWD_UP] = pantographs[0]->getSoundSignal(Pantograph::UP_SOUND);
    analogSignal[SOUND_PANT_FWD_DOWN] = pantographs[0]->getSoundSignal(Pantograph::DOWN_SOUND);
    // Главный выключатель
    analogSignal[SOUND_GV_ON] = main_switch->getSoundSignal(ProtectiveDevice::ON_SOUND);
    analogSignal[SOUND_GV_OFF] = main_switch->getSoundSignal(ProtectiveDevice::OFF_SOUND);
    // Трансформатор
    analogSignal[SOUND_TRANSFORMER] = trac_trans->getSoundSignal();

    // Песочница
    analogSignal[SOUND_SAND_DELIVERY] = sand_system->getSoundSignal();
    // Мотор-вентиляторы
    analogSignal[SOUND_FAN6] = motor_fans[MV6]->getSoundSignal();
    analogSignal[SOUND_FAN5] = motor_fans[MV5]->getSoundSignal();
    analogSignal[SOUND_FAN4] = motor_fans[MV4]->getSoundSignal();
    analogSignal[SOUND_FAN3] = motor_fans[MV3]->getSoundSignal();
    analogSignal[SOUND_FAN2] = motor_fans[MV2]->getSoundSignal();
    analogSignal[SOUND_FAN1] = motor_fans[MV1]->getSoundSignal();
    // Мотор-компрессор
    analogSignal[SOUND_COMPR1] = motor_compressor->getSoundSignal();
    analogSignal[SOUND_COMPR2] = motor_compressor->getSoundSignal();
    // Фазорасщепитель
    analogSignal[SOUND_PHASESPLITTER1] = phase_spliter->getSoundSignal();
    analogSignal[SOUND_PHASESPLITTER2] = phase_spliter->getSoundSignal();

    // Тяговые электродвигатели
    analogSignal[SOUND_TRACTION_ELETROENGINE_1] = motor[TED1]->getSoundSignal();
    analogSignal[SOUND_TRACTION_ELETROENGINE_2] = motor[TED2]->getSoundSignal();
    analogSignal[SOUND_TRACTION_ELETROENGINE_3] = motor[TED3]->getSoundSignal();
    analogSignal[SOUND_TRACTION_ELETROENGINE_4] = motor[TED4]->getSoundSignal();
    analogSignal[SOUND_TRACTION_ELETROENGINE_5] = motor[TED5]->getSoundSignal();
    analogSignal[SOUND_TRACTION_ELETROENGINE_6] = motor[TED6]->getSoundSignal();

    // Серводвигатель ЭКГ, ручное и автоматическое движение
    analogSignal[SOUND_EKG_ONE] = main_controller->getSoundSignal(EKG_8G::CHANGE_POS_ONE_SOUND);
    analogSignal[SOUND_EKG_AUTO] = main_controller->getSoundSignal(EKG_8G::CHANGE_POS_AUTO_SOUND);
}
