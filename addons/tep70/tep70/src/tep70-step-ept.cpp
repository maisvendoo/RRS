#include    "tep70.h"

void TEP70::stepEPT(double t, double dt)
{
    ept_converter->setU_bat(Ucc);
    ept_converter->setI_out(ept_current[0]);
    ept_converter->step(t, dt);

    ept_pass_control->setU(ept_converter->getU_out() * static_cast<double>(azv_ept_on.getState()));
    ept_pass_control->setHoldState(krm->isHold());
    ept_pass_control->setBrakeState(krm->isBrake());
    ept_pass_control->step(t, dt);

    bool is_EPT_on = button_brake_release;

    ept_control[0] = ept_pass_control->getControlSignal() * static_cast<double>(is_EPT_on);

    if (prev_vehicle != nullptr)
    {
        ept_control[0] = prev_vehicle->getEPTControl(0);
    }

    ept_current[0] = 0;

    if (next_vehicle != nullptr)
    {
        ept_current[0] += next_vehicle->getEPTCurrent(0);
        next_vehicle->setEPTControl(0, ept_control[0]);
    }

    ept_current[0] += evr->getValveState(0) + evr->getValveState(1);
}
