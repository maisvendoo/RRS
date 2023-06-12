#include    "passcar.h"
#include    "passcar-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(t);
    Q_UNUSED(dt);

    DebugMsg = QString("t%1 s|")
            .arg(t, 7, 'f', 1);
    DebugMsg += QString("x%1 km|V%2 km/h|")
            .arg(railway_coord / 1000.0, 8, 'f', 3)
            .arg(velocity * Physics::kmh, 6, 'f', 1);
    DebugMsg += QString("pBP%1 MPa|pBC%2 MPa|pSR%3 MPa|")
            .arg(brakepipe->getPressure(), 7, 'f', 3)
            .arg(brake_mech->getBCpressure(), 7, 'f', 3)
            .arg(supply_reservoir->getPressure(), 7, 'f', 3);
    DebugMsg += QString("          ");
}
