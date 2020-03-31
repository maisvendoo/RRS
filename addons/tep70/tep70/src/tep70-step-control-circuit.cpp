#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepControlCircuit(double t, double dt)
{
    Ucc = max(battery->getVoltage(), 0.0);

    battery->setLoadCurrent(100.0);
    battery->step(t, dt);
}
