#include    "vl60k.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::debugPrint(double t, double dt)
{
    Q_UNUSED(dt)

    DebugMsg = QString("t%1 s|")
            .arg(t, 7, 'f', 1);
    DebugMsg += QString("x%1 km|V%2 km/h|")
            .arg(railway_coord / 1000.0, 8, 'f', 3)
            .arg(velocity * Physics::kmh, 6, 'f', 1);
    DebugMsg += QString("pBP%1|pBC%2|pSR%3|")
            .arg(10.0 * brakepipe->getPressure(), 6, 'f', 2)
            .arg(10.0 * brake_mech[TROLLEY_FWD]->getBCpressure(), 6, 'f', 2)
            .arg(10.0 * supply_reservoir->getPressure(), 6, 'f', 2);
    DebugMsg += QString("pFL%1|pER%2|395:%3|254:%4%|")
            .arg(10.0 * main_reservoir->getPressure(), 6, 'f', 2)
            .arg(10.0 * brake_crane->getERpressure(), 6, 'f', 2)
            .arg(brake_crane->getPositionName(), 3)
            .arg(loco_crane->getHandlePosition() * 100.0, 3, 'f', 0);
    DebugMsg += QString("Rev%1|Pos %2%3|I%4 A|")
            .arg(controller->getReversHandlePos() * 4.0, 2, 'f', 0)
            .arg(main_controller->getPosition(), 2)
            .arg(main_controller->isLongMotionPos() ? "*" : " ")
            .arg(motor[TED1]->getIa(), 6, 'f', 1);
    DebugMsg += QString("F%1|B%2|")
            .arg(oper_rod_fwd->getOperatingState(), 5, 'f', 2)
            .arg(oper_rod_bwd->getOperatingState(), 5, 'f', 2);
    DebugMsg += QString("hoseB %1|angle:down%2°,side%3°")
                    .arg(hose_bp_bwd->isConnected())
                    .arg(hose_bp_bwd->getDownAngle()*180.0/Physics::PI, 7, 'f', 2)
                    .arg(hose_bp_bwd->getSideAngle()*180.0/Physics::PI, 7, 'f', 2);
    DebugMsg += QString("          ");
}
