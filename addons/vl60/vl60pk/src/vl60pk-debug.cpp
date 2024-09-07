#include    "vl60pk.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::debugPrint(double t, double dt)
{
    (void) t;
    (void) dt;

    DebugMsg = "";
    DebugMsg += QString("x%1 km|V%2 km/h|")
                    .arg(profile_point_data.railway_coord / 1000.0, 10, 'f', 3)
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
    DebugMsg += QString("BP:Anglecock F%1 B%2|Hose F%3 B%4|")
                    .arg(anglecock_bp_fwd->isOpened())
                    .arg(anglecock_bp_bwd->isOpened())
                    .arg(hose_bp_fwd->isConnected())
                    .arg(hose_bp_bwd->isConnected());

    DebugMsg += QString("| Curvature: %1").arg(profile_point_data.curvature, 8, 'f', 6);

    if (profile_point_data.curvature > Physics::ZERO)
    {
        DebugMsg += QString("| Radius: %1").arg(1.0 / profile_point_data.curvature, 8, 'f', 1);
    }
    else
    {
        DebugMsg += QString("| Radius: inf");
    }

    DebugMsg += QString("\n");
    DebugMsg += QString("Traj coord at +%1m:%2")
                    .arg(length / 2.0, 4, 'f', 1)
                    .arg(speedmap_fwd->getCurrentLimit(), 8, 'f', 1);
    DebugMsg += QString(" | ");
    DebugMsg += QString("Traj coord at -%1m:%2")
                    .arg(length / 2.0, 4, 'f', 1)
                    .arg(speedmap_bwd->getCurrentLimit(), 8, 'f', 1);
}
