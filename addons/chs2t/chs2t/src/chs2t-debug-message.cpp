#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(dt)

    DebugMsg = QString("t%1 s|")
            .arg(t, 7, 'f', 1);
    DebugMsg += QString("x%1 km|V%2 km/h|")
            .arg(railway_coord / 1000.0, 8, 'f', 3)
            .arg(velocity * Physics::kmh, 6, 'f', 1);
    DebugMsg += QString("pBP%1 MPa|pBC%2 MPa|pSR%3 MPa|")
            .arg(brakepipe->getPressure(), 7, 'f', 3)
            .arg(brake_mech[TROLLEY_FWD]->getBCpressure(), 7, 'f', 3)
            .arg(supply_reservoir->getPressure(), 7, 'f', 3);
    DebugMsg += QString("395:%1|254:%2%|")
            .arg(brake_crane->getPositionName(), 3)
            .arg(loco_crane->getHandlePosition() * 100.0, 3, 'f', 0);
    DebugMsg += QString("Rev%1|Pos %2%3|I%4 A|")
            .arg(stepSwitch->getReverseState(), 2)
            .arg(stepSwitch->getPoz(), 2)
            .arg(stepSwitch->getHod() ? "*" : " ")
            .arg(motor->getI56(), 6, 'f', 1);
    DebugMsg += QString("ALSN:%1|D:%2|")
            .arg(alsn_info.code_alsn, 2)
            .arg(alsn_info.signal_dist, 8, 'f', 1);
    DebugMsg += QString("          ");
}
