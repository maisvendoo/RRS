#include    "freightcar.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::stepDebugMsg(double t, double dt)
{
    (void) t;
    (void) dt;

    DebugMsg = "";
    DebugMsg += QString("x%1 km|V%2 km/h|")
                    .arg(railway_coord / 1000.0, 8, 'f', 3)
                    .arg(velocity * Physics::kmh, 6, 'f', 1);
    DebugMsg += QString("pBP%1|pBC%2|pSR%3|")
                    .arg(10.0 * brakepipe->getPressure(), 6, 'f', 2)
                    .arg(10.0 * brake_mech->getBCpressure(), 6, 'f', 2)
                    .arg(10.0 * supply_reservoir->getPressure(), 6, 'f', 2);
    if (automode != nullptr)
    {
        DebugMsg += QString("pAutoMode%1(%2%)|")
                        .arg(10.0 * automode->getAirDistBCpressure(), 6, 'f', 2)
                        .arg(100.0 * payload_coeff, 3, 'f', 0);
    }
    DebugMsg += QString("BP:Anglecock F%1 B%2|Hose F%3 B%4|")
                    .arg(anglecock_bp_fwd->isOpened())
                    .arg(anglecock_bp_bwd->isOpened())
                    .arg(hose_bp_fwd->isConnected())
                    .arg(hose_bp_bwd->isConnected());
}
