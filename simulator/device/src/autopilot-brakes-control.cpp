#include    <autopilot-brakes-control.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::step_control(bool is_EPB_ON,
                                            double dv,
                                            bool is_motion_allowed,
                                            bool &lock_traction,
                                            bool &is_disable_release)
{
    if (is_EPB_ON)
    {
        stepEPB(dv, lock_traction, is_disable_release);
    }
    else
    {
        stepPB(dv, lock_traction, is_disable_release);
    }

    stepKVT(is_motion_allowed, is_disable_release);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::stepEPB(double dv,
                                       bool &lock_traction,
                                       bool &is_disable_release)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::stepPB(double dv,
                                      bool &lock_traction,
                                      bool &is_disable_release)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AutopilotBrakeController::stepKVT(bool is_motion_allowed,
                                       bool &is_disable_release)
{

}
