#include    "tep70.h"

void TEP70::stepMSUTsignals(double t, double dt)
{
    msut_input_t msut_input;

    msut_input.velocity = velocity;
    msut_input.is_KP1_KP6_on = is_KP1_KP6_on;
    msut_input.bc_pressure = fwd_trolley->getBrakeCylinderPressure();

    msut->setInputData(msut_input);

    msut_output_t msut_output = msut->getOutputData();

    analogSignal[MSUT_REVERSOR] = reversor->getSatate();
    analogSignal[MSUT_SPEED] = velocity * 3.6;

    analogSignal[MSUT_VU2_I] = 0;
    analogSignal[MSUT_VU2_U] = 0;

    analogSignal[MSUT_VU1_I] = I_gen / 1000.0;
    analogSignal[MSUT_VU1_U] = trac_gen->getVoltage() / 1000.0;
    analogSignal[MSUT_VU1_I_TED] = motor[0]->getAncorCurrent() / 1000.0;

    analogSignal[MSUT_POSITION] = km->getPositionNumber();

    double power_kW = I_gen * trac_gen->getVoltage() / 1000.0;
    analogSignal[MSUT_POWER1] = power_kW;

    analogSignal[MSUT_ACCELLERATION] = msut_output.acceleration;

    double traction = 0;

    for (size_t i = 1; i < Q_a.size(); ++i)
    {
        traction += 2 * Q_a[i] / wheel_diameter;
    }

    analogSignal[MSUT_ET_T] = traction / Physics::g / 1000.0;

    analogSignal[MSUT_MODE] = msut_output.mode;
}
