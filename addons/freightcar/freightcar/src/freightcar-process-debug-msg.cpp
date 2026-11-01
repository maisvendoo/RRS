#include "freightcar-signals.h"
#include    "freightcar.h"

#include "brake-auto-mode.h"
#include "brake-mech.h"
#include "coupling.h"
#include "coupling-operating-rod.h"
#include "pneumo-anglecock.h"
#include "pneumo-hose-epb.h"
#include "reservoir.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::debugPrint(const simulator_time_t& t, const double& dt)
{
    (void) t;
    (void) dt;

    DebugMsg = "";
    DebugMsg += QString("x%1 km|V%2 km/h|")
                    .arg(profile_point_data.railway_coord / 1000.0, 10, 'f', 3)
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
    DebugMsg += QString("\n");
    DebugMsg += QString("%1%2%3-%4-couplings-%5-%6%7%8")
                    .arg(coupling_fwd->isLinked() ? "=" : " ")
                    .arg(coupling_fwd->isCoupled() ? "=" : " ")
                    .arg((coupling_fwd->getOutputSignal(COUPL_OUTPUT_REF_STATE) > -0.5) ? "=" : ">")
                    .arg((oper_rod_fwd->getOperatingState() > -0.5) ? "|" : "/")
                    .arg((oper_rod_bwd->getOperatingState() > -0.5) ? "|" : "\\")
                    .arg((coupling_bwd->getOutputSignal(COUPL_OUTPUT_REF_STATE) > -0.5) ? "=" : "<")
                    .arg(coupling_bwd->isCoupled() ? "=" : " ")
                    .arg(coupling_bwd->isLinked() ? "=" : " ");
    DebugMsg += QString("  |  ");
    DebugMsg += QString("%1%2/=%3==BP==%4=\\%5%6")
                    .arg(hose_bp_fwd->isLinked() ? "\\" : " ")
                    .arg(hose_bp_fwd->isConnected() ? "_" : " ")
                    .arg(anglecock_bp_fwd->isOpened() ? "/" : "|")
                    .arg(anglecock_bp_bwd->isOpened() ? "\\" : "|")
                    .arg(hose_bp_bwd->isConnected() ? "_" : " ")
                    .arg(hose_bp_bwd->isLinked() ? "/" : " ");
    DebugMsg += QString("  |  ");
    DebugMsg += QString("down %1(%2) side %3(%4) up %5(%6)")
                    .arg(hose_bp_fwd->getDownAngle() * 180.0/Physics::PI + 40.894, 6, 'f', 2)
                    .arg((analogSignal[HOSE_BP_FWD_ANGLE_DOWN]-1) * 180.0/Physics::PI, 6, 'f', 2)
                    .arg(hose_bp_fwd->getSideAngle() * 180.0/Physics::PI - 20.704, 6, 'f', 2)
                    .arg((analogSignal[HOSE_BP_FWD_ANGLE_SIDE]-1) * 180.0/Physics::PI, 6, 'f', 2)
                    .arg(hose_bp_fwd->getHeadUpAngle() * 180.0/Physics::PI + 24, 6, 'f', 2)
                    .arg((analogSignal[HOSE_BP_FWD_ANGLE_HEAD_UP]-1) * 180.0/Physics::PI, 6, 'f', 2);
    DebugMsg += QString("{%1, %2, %3}")
                    .arg(hose_bp_fwd->getConnectionPoint().x, 7, 'f', 3)
                    .arg(hose_bp_fwd->getConnectionPoint().y, 7, 'f', 3)
                    .arg(hose_bp_fwd->getConnectionPoint().z, 7, 'f', 3);
    DebugMsg += QString("  |  ");
    DebugMsg += QString("down %1(%2) side %3(%4) up %5(%6)")
                    .arg((hose_bp_bwd->getDownAngle()) * 180.0/Physics::PI + 40.894, 6, 'f', 2)
                    .arg((analogSignal[HOSE_BP_BWD_ANGLE_DOWN]-1) * 180.0/Physics::PI, 6, 'f', 2)
                    .arg((hose_bp_bwd->getSideAngle()) * 180.0/Physics::PI - 20.704, 6, 'f', 2)
                    .arg((analogSignal[HOSE_BP_BWD_ANGLE_SIDE]-1) * 180.0/Physics::PI, 6, 'f', 2)
                    .arg((hose_bp_bwd->getHeadUpAngle()) * 180.0/Physics::PI + 24, 6, 'f', 2)
                    .arg((analogSignal[HOSE_BP_BWD_ANGLE_HEAD_UP]-1) * 180.0/Physics::PI, 6, 'f', 2);
    DebugMsg += QString("{%1, %2, %3}")
                    .arg(hose_bp_bwd->getConnectionPoint().x, 7, 'f', 3)
                    .arg(hose_bp_bwd->getConnectionPoint().y, 7, 'f', 3)
                    .arg(hose_bp_bwd->getConnectionPoint().z, 7, 'f', 3);

    if (brake_shoes_set.getState())
    {
        DebugMsg += QString("  |  ");
        DebugMsg += QString("!!! УСТАНОВЛЕНЫ БАШМАКИ !!! Торм. усил.: %1 кН")
                        .arg(shoesForce / 1000.0, 6, 'f', 2);
    }
}
