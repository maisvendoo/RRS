#include    "vl60k.h"

#include "vl60k-signals.h"

#include "alsn-ukbm.h"
#include "brake-crane.h"
#include "brake-lock.h"
#include "dc-motor.h"
#include "ekg-8g.h"
#include "kme-60-044.h"
#include "loco-crane.h"
#include "motor-fan-ac.h"
#include "oscillator.h"
#include "pantograph.h"
#include "phase-splitter.h"
#include "pneumo-splitter.h"
#include "protective-device.h"
#include "reservoir.h"
#include "sl2m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::stepSignalsOutput(double t, double dt)
{
    // Состояние токоприемников
    analogSignal[PANT1_POS] = static_cast<float>(pantographs[0]->getHeight());
    analogSignal[PANT2_POS] = static_cast<float>(pantographs[1]->getHeight());

    // Состояние тумблеров на пульте машиниста
    analogSignal[TUMBLER_PNT] = static_cast<float>(pants_tumbler[CAB1].getState());
    analogSignal[TUMBLER_PNT1] = static_cast<float>(pant1_tumbler[CAB1].getState());
    analogSignal[TUMBLER_PNT2] = static_cast<float>(pant2_tumbler[CAB1].getState());

    analogSignal[CAB2_TUMBLER_PNT] = static_cast<float>(pants_tumbler[CAB2].getState());
    analogSignal[CAB2_TUMBLER_PNT1] = static_cast<float>(pant1_tumbler[CAB2].getState());
    analogSignal[CAB2_TUMBLER_PNT2] = static_cast<float>(pant2_tumbler[CAB2].getState());

    analogSignal[TUMBLER_GV_ON] = static_cast<float>(gv_return_tumbler[CAB1].getState());
    analogSignal[TUMBLER_GV_ON_OFF] = static_cast<float>(gv_tumbler[CAB1].getState());

    analogSignal[CAB2_TUMBLER_GV_ON] = static_cast<float>(gv_return_tumbler[CAB2].getState());
    analogSignal[CAB2_TUMBLER_GV_ON_OFF] = static_cast<float>(gv_tumbler[CAB2].getState());

    analogSignal[TUMBLER_FR] = static_cast<float>(fr_tumbler[CAB1].getState());

    analogSignal[TUMBLER_MV1] = static_cast<float>(mv_tumblers[CAB1][MV1].getState());
    analogSignal[TUMBLER_MV2] = static_cast<float>(mv_tumblers[CAB1][MV2].getState());
    analogSignal[TUMBLER_MV3] = static_cast<float>(mv_tumblers[CAB1][MV3].getState());
    analogSignal[TUMBLER_MV4] = static_cast<float>(mv_tumblers[CAB1][MV4].getState());
    analogSignal[TUMBLER_MV5] = static_cast<float>(mv_tumblers[CAB1][MV5].getState());
    analogSignal[TUMBLER_MV6] = static_cast<float>(mv_tumblers[CAB1][MV6].getState());

    analogSignal[TUMBLER_MK] = static_cast<float>(mk_tumbler[CAB1].getState());

    analogSignal[TUMBLER_CU] = static_cast<float>(cu_tumbler[CAB1].getState());
    analogSignal[CAB2_TUMBLER_CU] = static_cast<float>(cu_tumbler[CAB2].getState());

    // Вольтметр КС
    analogSignal[STRELKA_KV2] = static_cast<float>(main_switch->getU_out() / 30000.0);

    // Вольтметр ТЭД
    analogSignal[STRELKA_KV1] = static_cast<float>(gauge_KV_motors->getOutput() / 3000.0);

    // Состояние главного выключателя
    analogSignal[GV_POS] = static_cast<float>(main_switch->getKnifePos());

    // Состояние локомотивного светофора
    stepSafetyDevices(t, dt);

    // Состояние контрольных ламп
    analogSignal[SIG_LIGHT_GV] = main_switch->getLampState();
    analogSignal[SIG_LIGHT_GU] = phase_spliter->isNotReady();
    analogSignal[SIG_LIGHT_FR] = phase_spliter->isNotReady();
    analogSignal[SIG_LIGHT_0HP] = static_cast<float>(main_controller->isLongMotionPos());
    analogSignal[SIG_LIGHT_TR] = static_cast<float>(!motor_fans[MV3]->isReady() || !motor_fans[MV4]->isReady());
    analogSignal[SIG_LIGHT_VU1] = static_cast<float>(!motor_fans[MV1]->isReady() || !motor_fans[MV2]->isReady());
    analogSignal[SIG_LIGHT_VU2] = static_cast<float>(!motor_fans[MV5]->isReady() || !motor_fans[MV6]->isReady());
    analogSignal[SIG_LIGHT_TD] = isLineContactorsOff();

    // Состояние КМ
    analogSignal[KONTROLLER] = controller[CAB1]->getMainHandlePos();
    analogSignal[REVERS] = controller[CAB1]->getReversHandlePos();

    analogSignal[CAB2_KONTROLLER] = controller[CAB2]->getMainHandlePos();
    analogSignal[CAB2_REVERS] = controller[CAB2]->getReversHandlePos();

    analogSignal[STRELKA_SELSIN] = main_controller->getSelsinPosition();
    analogSignal[CAB2_STRELKA_SELSIN] = main_controller->getSelsinPosition();

    // Положение рукоятки комбинированного крана
    analogSignal[KRAN_KOMBIN] = brake_lock[CAB1]->getCombCranePosition();
    analogSignal[CAB2_KRAN_KOMBIN] = brake_lock[CAB2]->getCombCranePosition();
    // Положение рукоятки УБТ
    analogSignal[KLUCH_367] = brake_lock[CAB1]->getMainHandlePosition();
    analogSignal[CAB2_KLUCH_367] = brake_lock[CAB2]->getMainHandlePosition();

    // Манометр питательной магистрали
    analogSignal[STRELKA_M_HM] = static_cast<float>(main_reservoir->getPressure() / 1.6);
    analogSignal[CAB2_STRELKA_M_HM] = static_cast<float>(main_reservoir->getPressure() / 1.6);
    // Манометр тормозной магистрали
    analogSignal[STRELKA_M_TM] = static_cast<float>(brakepipe->getPressure() / 1.0);
    analogSignal[CAB2_STRELKA_M_TM] = static_cast<float>(brakepipe->getPressure() / 1.0);
    // Манометр уравнительного резервуара
    analogSignal[STRELKA_M_UR] = static_cast<float>(brake_crane[CAB1]->getERpressure() / 1.0);
    analogSignal[CAB2_STRELKA_M_UR] = static_cast<float>(brake_crane[CAB2]->getERpressure() / 1.0);
    // Манометр давления в ТЦ
    analogSignal[STRELKA_M_TC] = static_cast<float>(bc_splitter->getInputPressure() / 1.0);
    analogSignal[CAB2_STRELKA_M_TC] = static_cast<float>(bc_splitter->getInputPressure() / 1.0);

    // Положение рукоятки КрМ
    analogSignal[KRAN395_RUK] = static_cast<float>(brake_crane[CAB1]->getHandlePosition());
    analogSignal[CAB2_KRAN395_RUK] = static_cast<float>(brake_crane[CAB2]->getHandlePosition());

    // Положение рукоятки КВТ
    analogSignal[KRAN254_RUK] = static_cast<float>(loco_crane[CAB1]->getHandlePosition());
    analogSignal[KRAN254_SHIFT] = static_cast<float>(loco_crane[CAB1]->getHandleShift());
    analogSignal[CAB2_KRAN254_RUK] = static_cast<float>(loco_crane[CAB2]->getHandlePosition());
    //analogSignal[CAB2_KRAN254_SHIFT] = static_cast<float>(loco_crane[CAB2]->getHandleShift());

    analogSignal[STRELKA_AMP1] = static_cast<float>(motor[TED1]->getIa() / 1500.0);
    analogSignal[STRELKA_AMP2] = static_cast<float>(motor[TED6]->getIa() / 1500.0);

    analogSignal[STRELKA_SPEED] = speed_meter->getArrowPos();
    analogSignal[VAL_PR_SKOR1] = speed_meter->getShaftPos();
    analogSignal[VAL_PR_SKOR2] = speed_meter->getShaftPos();

    analogSignal[CAB2_STRELKA_SPEED] = speed_meter->getArrowPos();
    analogSignal[CAB2_VAL_PR_SKOR1] = speed_meter->getShaftPos();
    analogSignal[CAB2_VAL_PR_SKOR2] = speed_meter->getShaftPos();

    analogSignal[KNOPKA_RB_1] = static_cast<float>(rb[CAB1][RB_1].getState());
    analogSignal[KNOPKA_RBS] = static_cast<float>(rb[CAB1][RBS].getState());
    analogSignal[KNOPKA_RBP] = static_cast<float>(rb[CAB1][RBP].getState());

    analogSignal[CAB2_KNOPKA_RB_1] = static_cast<float>(rb[CAB2][RB_1].getState());
    analogSignal[CAB2_KNOPKA_RBS] = static_cast<float>(rb[CAB2][RBS].getState());
    analogSignal[CAB2_KNOPKA_RBP] = static_cast<float>(rb[CAB2][RBP].getState());

    analogSignal[WHEEL_1] = static_cast<float>(wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(wheel_rotation_angle[3] / 2.0 / Physics::PI);
    analogSignal[WHEEL_5] = static_cast<float>(wheel_rotation_angle[4] / 2.0 / Physics::PI);
    analogSignal[WHEEL_6] = static_cast<float>(wheel_rotation_angle[5] / 2.0 / Physics::PI);

    // Лампы предварительной световой сигнализации УКБМ
    analogSignal[SIG_LIGHT_PSS_L] = safety_device[CAB1]->getStatePSS();
    analogSignal[SIG_LIGHT_PSS_R] = safety_device[CAB1]->getStatePSS();
    analogSignal[CAB2_SIG_LIGHT_PSS_L] = safety_device[CAB2]->getStatePSS();
    analogSignal[CAB2_SIG_LIGHT_PSS_R] = safety_device[CAB2]->getStatePSS();
    analogSignal[SIG_LIGHT_PSS_PROPUSK] = 0.0f;

    // Лампы локомотивного светофора
    analogSignal[LS_W] = safety_device[CAB1]->getWhiteLamp();
    analogSignal[LS_YK] = safety_device[CAB1]->getRedYellowLamp();
    analogSignal[LS_R] = safety_device[CAB1]->getRedLamp();
    analogSignal[LS_Y] = safety_device[CAB1]->getYellowLamp();
    analogSignal[LS_G] = safety_device[CAB1]->getGreenLamp();

    analogSignal[KLUCH_EPK] = static_cast<float>(key_epk[CAB1].getState());
}
