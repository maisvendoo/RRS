#include    "tep70.h"

void TEP70::stepMSUTsignals(double t, double dt)
{
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
}
