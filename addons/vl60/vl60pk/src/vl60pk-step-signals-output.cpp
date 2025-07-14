#include    "vl60pk.h"

#include "alsn-ukbm.h"
#include "brake-crane.h"
#include "brake-lock.h"
#include "brake-mech.h"
#include "dc-motor.h"
#include "ekg-8g.h"
#include "epb-2line-control.h"
#include "epb-converter.h"
#include "kme-60-044.h"
#include "loco-crane.h"
#include "motor-fan-ac.h"
#include "oscillator.h"
#include "pantograph.h"
#include "phase-splitter.h"
#include "protective-device.h"
#include "reservoir.h"
#include "sl2m.h"
#include "vl60pk-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepSignalsOutput(double t, double dt)
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

    analogSignal[TUMBLER_GV_ON] = static_cast<float>(gv_return_tumbler.getState());
    analogSignal[TUMBLER_GV_ON_OFF] = static_cast<float>(gv_tumbler.getState());

    analogSignal[TUMBLER_FR] = static_cast<float>(fr_tumbler.getState());

    analogSignal[TUMBLER_MV1] = static_cast<float>(mv_tumblers[MV1].getState());
    analogSignal[TUMBLER_MV2] = static_cast<float>(mv_tumblers[MV2].getState());
    analogSignal[TUMBLER_MV3] = static_cast<float>(mv_tumblers[MV3].getState());
    analogSignal[TUMBLER_MV4] = static_cast<float>(mv_tumblers[MV4].getState());
    analogSignal[TUMBLER_MV5] = static_cast<float>(mv_tumblers[MV5].getState());
    analogSignal[TUMBLER_MV6] = static_cast<float>(mv_tumblers[MV6].getState());

    analogSignal[TUMBLER_MK] = static_cast<float>(mk_tumbler.getState());

    analogSignal[TUMBLER_CU] = static_cast<float>(cu_tumbler.getState());

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

    analogSignal[KONTROLLER] = controller->getMainHandlePos();
    analogSignal[REVERS] = controller->getReversHandlePos();
    analogSignal[STRELKA_SELSIN] = main_controller->getSelsinPosition();

    // Положение рукоятки комбинированного крана
    analogSignal[KRAN_KOMBIN] = brake_lock->getCombCranePosition();
    // Положение рукоятки УБТ
    analogSignal[KLUCH_367] = brake_lock->getMainHandlePosition();

    analogSignal[STRELKA_AMP_EPT] = static_cast<float>(abs(epb_converter->getOutputCurrent()) / 10.0);
    analogSignal[SIG_LIGHT_O] = static_cast<float>(epb_control->stateReleaseLamp());
    analogSignal[SIG_LIGHT_P] = static_cast<float>(epb_control->stateHoldLamp());
    analogSignal[SIG_LIGHT_T] = static_cast<float>(epb_control->stateBrakeLamp());
    analogSignal[EPT_ON_OFF] = static_cast<float>(epb_switch.getState());

    // Манометр питательной магистрали
    analogSignal[STRELKA_M_HM] = static_cast<float>(main_reservoir->getPressure() / 1.6);
    // Манометр тормозной магистрали
    analogSignal[STRELKA_M_TM] = static_cast<float>(brakepipe->getPressure() / 1.0);
    // Манометр уравнительного резервуара
    analogSignal[STRELKA_M_UR] = static_cast<float>(brake_crane->getERpressure() / 1.0);
    // Манометр давления в ТЦ
    analogSignal[STRELKA_M_TC] = static_cast<float>(brake_mech[TROLLEY_BWD]->getBCpressure() / 1.0);

    // Положение рукоятки КрМ
    analogSignal[KRAN395_RUK] = static_cast<float>(brake_crane->getHandlePosition());

    // Положение рукоятки КВТ
    analogSignal[KRAN254_RUK] = static_cast<float>(loco_crane->getHandlePosition());
    analogSignal[KRAN254_SHIFT] = static_cast<float>(loco_crane->getHandleShift());

    analogSignal[STRELKA_AMP1] = static_cast<float>(motor[TED1]->getIa() / 1500.0);
    analogSignal[STRELKA_AMP2] = static_cast<float>(motor[TED6]->getIa() / 1500.0);

    analogSignal[STRELKA_SPEED] = speed_meter->getArrowPos();
    analogSignal[VAL_PR_SKOR1] = speed_meter->getShaftPos();
    analogSignal[VAL_PR_SKOR2] = speed_meter->getShaftPos();

    analogSignal[KNOPKA_RB_1] = static_cast<float>(rb[RB_1].getState());
    analogSignal[KNOPKA_RBS] = static_cast<float>(rb[RBS].getState());
    analogSignal[KNOPKA_RBP] = static_cast<float>(rb[RBP].getState());

    analogSignal[WHEEL_1] = static_cast<float>(wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(wheel_rotation_angle[3] / 2.0 / Physics::PI);
    analogSignal[WHEEL_5] = static_cast<float>(wheel_rotation_angle[4] / 2.0 / Physics::PI);
    analogSignal[WHEEL_6] = static_cast<float>(wheel_rotation_angle[5] / 2.0 / Physics::PI);

    // Лампы предварительной световой сигнализации УКБМ
    analogSignal[SIG_LIGHT_PSS_L] = safety_device->getStatePSS();
    analogSignal[SIG_LIGHT_PSS_R] = safety_device->getStatePSS();
    analogSignal[SIG_LIGHT_PSS_PROPUSK] = 0.0f;

    // Лампы локомотивного светофора
    analogSignal[LS_W] = safety_device->getWhiteLamp();
    analogSignal[LS_YK] = safety_device->getRedYellowLamp();
    analogSignal[LS_R] = safety_device->getRedLamp();
    analogSignal[LS_Y] = safety_device->getYellowLamp();
    analogSignal[LS_G] = safety_device->getGreenLamp();

    analogSignal[KLUCH_EPK] = static_cast<float>(key_epk.getState());
}
