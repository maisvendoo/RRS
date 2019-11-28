#include    "tep70.h"


void TEP70::stepSignalsOutput(double t, double dt)
{
    analogSignal[KM_SHTURVAL] = km->getMainShaftPos();
    analogSignal[KM_REVERSOR] = km->getReversState();
}
