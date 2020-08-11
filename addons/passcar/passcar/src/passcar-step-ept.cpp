#include    "passcar.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCarrige::stepEPT(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    ept_control[0] = 0;

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

    electroAirDist->setControlLine(ept_control[0]);

    ept_current[0] += electroAirDist->getValveState(0) +
                      electroAirDist->getValveState(1);
}
