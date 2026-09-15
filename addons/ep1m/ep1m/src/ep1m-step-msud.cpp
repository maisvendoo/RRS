#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepMSUD(const double& t, const double& dt)
{
    bool is_MSUD_on = km43->getContactState(1);

    if (is_MSUD_on)
    {
        auto cab_state_to_msud_input = [&](std::size_t cab_idx)
        {
            msud_input.tumbler_MPK = tumblers[TUMBLER_MPK][cab_idx].getState();
            msud_input.is_PCHF_On = tumblers[TUMBLER_PCHF][cab_idx].getState();

            msud_input.is_auto_reg = tumblers[TUMBLER_AUTO_MODE][cab_idx].getState();
            msud_input.km_trac_level = km[cab_idx]->getTracLevel();
            msud_input.km_brake_level = km[cab_idx]->getBrakeLevel();
            msud_input.km_ref_velocity_level = km[cab_idx]->getRefSpeedLevel();

            msud_input.is_emergency_brake = (brake_crane[cab_idx]->getPositionName() == "VI") ||
                                            epk[cab_idx]->getEmergencyBrakeContact();
        };

        if (tumblers_panel[CAB1]->getTumblerState(EP1MTumblersPanel::TUMBLER_MSUD))
        {
            // Состояние из кабины 1
            cab_state_to_msud_input(CAB1);
        }

        if (tumblers_panel[CAB2]->getTumblerState(EP1MTumblersPanel::TUMBLER_MSUD))
        {
            // Состояние из кабины 2
            cab_state_to_msud_input(CAB2);
        }

        for (size_t i = 0; i < motor_fan.size(); ++i)
            msud_input.mv_state[i] = !motor_fan[i]->isNoReady();

        // Мерям токи тяговых двигателей
        for (size_t i = 0; i < msud_input.Ia.size(); ++i)
        {
            msud_input.Ia[i] = trac_motor[i]->getAncorCurrent();
            msud_input.If[i] = trac_motor[i]->getFieldCurrent();
        }

        for (size_t i = 0; i < brake_mech.size(); ++i)
        {
            msud_input.TC_press[i] = brake_mech[i]->getBCpressure();
        }

        msud_input.V_cur = std::abs(wheel_omega[0] * wheel_diameter[0] * Physics::kmh / 2.0);
    }

    msud->setPowerVoltage(Ucc * static_cast<double>(is_MSUD_on));
    msud->setInputData(msud_input);
    msud->step(t, dt);
}
