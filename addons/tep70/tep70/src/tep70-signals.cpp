#include    "tep70.h"


void TEP70::stepSignalsOutput(double t, double dt)
{
    analogSignal[KM_SHTURVAL] = km->getMainShaftPos();
    analogSignal[KM_REVERSOR] = km->getReversState();

    analogSignal[BUTTON_DISEL_START] = static_cast<float>(button_disel_start);
    analogSignal[BUTTON_BRAKE_RELEASE] = static_cast<float>(button_brake_release);
    analogSignal[BUTTON_SVISTOK] = static_cast<float>(button_svistok);
    analogSignal[BUTTON_TIFON] = static_cast<float>(button_tifon);

    analogSignal[AZV_COMMON_CONTROL] = static_cast<float>(azv_common_control.getState());
    analogSignal[AZV_UPR_TEPL] = static_cast<float>(azv_upr_tepl.getState());
    analogSignal[AZV_FUEL_PUMP] = static_cast<float>(azv_fuel_pump.getState());
    analogSignal[AZV_EDT_ON] = static_cast<float>(azv_edt_on.getState());
    analogSignal[AZV_EDT_POWER] = static_cast<float>(azv_edt_power.getState());
    analogSignal[AZV_EPT_ON] = static_cast<float>(azv_ept_on.getState());

    analogSignal[STRELKA_UR] = 0.0;
    analogSignal[STRELKA_TM] = 0.0;
    analogSignal[STRELKA_TC1] = 0.0;

    analogSignal[STRELKA_REOSTATE_CURRENT] = 0.0;
    analogSignal[STRELKA_GEN_CURRENT] = 0.0;

    analogSignal[STRELKA_WATER_TEMP] = 0.0;
    analogSignal[STRELKA_OIL_PRESS] = 0.0;

    analogSignal[WHEEL_1] = static_cast<float>(dir * wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(dir * wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(dir * wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(dir * wheel_rotation_angle[3] / 2.0 / Physics::PI);
    analogSignal[WHEEL_5] = static_cast<float>(dir * wheel_rotation_angle[4] / 2.0 / Physics::PI);
    analogSignal[WHEEL_6] = static_cast<float>(dir * wheel_rotation_angle[5] / 2.0 / Physics::PI);
}
