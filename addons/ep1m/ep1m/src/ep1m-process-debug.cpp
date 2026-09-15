#include    "ep1m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::debugPrint(const simulator_time_t& t, const double& dt)
{
    (void) t;
    (void) dt;

    DebugMsg = "";
    DebugMsg += QString("CABINE 1|");
    if (brake_lock[CAB1]->isStateOn())
    {
        DebugMsg += QString("367comb:%1|395:%2|pER%3|254:%4%|")
                        .arg(brake_lock[CAB1]->getCombineCraneHandlePosition(), 2, 'f', 0)
                        .arg(brake_crane[CAB1]->getPositionName(), 3)
                        .arg(10.0 * brake_crane[CAB1]->getERpressure(), 6, 'f', 2)
                        .arg(loco_crane[CAB1]->getHandlePosition() * 100.0, 3, 'f', 0);
    }
    else
    {
        DebugMsg += QString("367comb:%1| BRAKE CRANES ARE LOCKED  |")
                        .arg(brake_lock[CAB1]->getCombineCraneHandlePosition(), 2, 'f', 0);
    }

    if (km[CAB1]->isReversHandle())
    {
        DebugMsg += QString("R:%1|T:%2%|")
                        .arg(km[CAB1]->getReversHandlePos(), 2, 'f', 0)
                        .arg(100.0 * km[CAB1]->getHandlePosition(), 4, 'f', 0);
        if (tumblers[TUMBLER_AUTO_MODE][CAB1].getState())
        {
            DebugMsg += QString("V:%1|")
                        .arg(140.0 * km[CAB1]->getRefSpeedLevel(), 3, 'f', 0);
        }
        else
        {
            DebugMsg += QString("f%1|")
                            .arg(msud->getOutputData().field_level, 4, 'f', 2);
        }
    }
    else
    {
        DebugMsg += QString(" NO REVERS HANDLE |");
    }

    if (tumblers_panel[CAB1]->isKey())
    {
        if (tumblers_panel[CAB1]->isKeyOn())
        {
            DebugMsg += QString("Tumblers:UNLOCK|");
        }
        else
        {
            DebugMsg += QString("Tumblers:LOCKED|");
        }
    }
    else
    {
        DebugMsg += QString("Tumblers:NO KEY|");
    }

    if (epk[CAB1]->isKeyOn())
    {
        if (klub_BEL/*[CAB1]*/->getEPKstate())
        {
            DebugMsg += QString("EPK:on|");
            DebugMsg += QString("limit %1km/h(%2km/h|%3m)|")
                            .arg(speedmap_fwd->getCurrentLimit(), 3, 'f', 0)
                            .arg(speedmap_fwd->getNextLimit(), 3, 'f', 0)
                            .arg(speedmap_fwd->getNextLimitDistance(), 6, 'f', 1);
        }
        else
        {
            if (epk[CAB1]->getEmergencyBrakeContact())
                DebugMsg += QString("EPK:EMERGENCY |");
            else
                DebugMsg += QString("EPK: WHISTLE  |");

            DebugMsg += QString("limit %1km/h|")
                            .arg(speedmap_fwd->getCurrentLimit(), 3, 'f', 0);
        }
        DebugMsg += QString("Code %1 (%2 Hz)| %3 (%4 m)")
                        .arg(coil_ALSN_fwd->getCode(), 1)
                        .arg(coil_ALSN_fwd->getFrequency(), 3, 'f', 0)
                        .arg(coil_ALSN_fwd->getNextSignalLiter())
                        .arg(coil_ALSN_fwd->getNextSignalDistance(), 6, 'f', 1);
    }
    else
    {
        if (epk[CAB1]->isKey())
        {
            DebugMsg += QString("EPK:OFF");
        }
        else
        {
            DebugMsg += QString("EPK:NO KEY");
        }
    }

    DebugMsg += QString("\n");
    DebugMsg += QString("x%1 km|V%2 km/h|")
                    .arg(profile_point_data.railway_coord / 1000.0, 10, 'f', 3)
                    .arg(velocity * Physics::kmh, 6, 'f', 1);
    DebugMsg += QString("pBP%1|pBC%2|pSR%3|pFL%4|")
                    .arg(10.0 * brakepipe->getPressure(), 6, 'f', 2)
                    .arg(10.0 * brake_mech[TROLLEY_FWD]->getBCpressure(), 6, 'f', 2)
                    .arg(10.0 * supply_reservoir->getPressure(), 6, 'f', 2)
                    .arg(10.0 * main_reservoir->getPressure(), 6, 'f', 2);
    DebugMsg += QString("I%3 A")
                    .arg(trac_motor[TRAC_MOTOR1]->getAncorCurrent(), 6, 'f', 0);

    DebugMsg += QString("  |  ");
    DebugMsg += QString("%1%2%3-%4-coupl-%5-%6%7%8")
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
    DebugMsg += QString("%1%2/=%3==FL==%4=\\%5%6")
                    .arg(hose_fl_fwd->isLinked() ? "\\" : " ")
                    .arg(hose_fl_fwd->isConnected() ? "_" : " ")
                    .arg(anglecock_fl_fwd->isOpened() ? "/" : "|")
                    .arg(anglecock_fl_bwd->isOpened() ? "\\" : "|")
                    .arg(hose_fl_bwd->isConnected() ? "_" : " ")
                    .arg(hose_fl_bwd->isLinked() ? "/" : " ");

    DebugMsg += QString("\n");
    DebugMsg += QString("CABINE 2|");
    if (brake_lock[CAB2]->isStateOn())
    {
        DebugMsg += QString("367comb:%1|395:%2|pER%3|254:%4%|")
                        .arg(brake_lock[CAB2]->getCombineCraneHandlePosition(), 2, 'f', 0)
                        .arg(brake_crane[CAB2]->getPositionName(), 3)
                        .arg(10.0 * brake_crane[CAB2]->getERpressure(), 6, 'f', 2)
                        .arg(loco_crane[CAB2]->getHandlePosition() * 100.0, 3, 'f', 0);
    }
    else
    {
        DebugMsg += QString("367comb:%1| BRAKE CRANES ARE LOCKED  |")
                        .arg(brake_lock[CAB2]->getCombineCraneHandlePosition(), 2, 'f', 0);
    }

    if (km[CAB2]->isReversHandle())
    {
        DebugMsg += QString("R:%1|T:%2%|")
                        .arg(km[CAB2]->getReversHandlePos(), 2, 'f', 0)
                        .arg(100.0 * km[CAB2]->getHandlePosition(), 4, 'f', 0);
        if (tumblers[TUMBLER_AUTO_MODE][CAB2].getState())
        {
            DebugMsg += QString("V:%1|")
                            .arg(140.0 * km[CAB2]->getRefSpeedLevel(), 3, 'f', 0);
        }
        else
        {
            DebugMsg += QString("f%1|")
                            .arg(msud->getOutputData().field_level, 4, 'f', 2);
        }
    }
    else
    {
        DebugMsg += QString(" NO REVERS HANDLE |");
    }

    if (tumblers_panel[CAB2]->isKey())
    {
        if (tumblers_panel[CAB2]->isKeyOn())
        {
            DebugMsg += QString("Tumblers:UNLOCK|");
        }
        else
        {
            DebugMsg += QString("Tumblers:LOCKED|");
        }
    }
    else
    {
        DebugMsg += QString("Tumblers:NO KEY|");
    }

    if (epk[CAB2]->isKeyOn())
    {
        if (klub_BEL/*[CAB2]*/->getEPKstate())
        {
            DebugMsg += QString("EPK:on|");
            DebugMsg += QString("limit %1km/h(%2km/h|%3m)|")
                            .arg(speedmap_bwd->getCurrentLimit(), 3, 'f', 0)
                            .arg(speedmap_bwd->getNextLimit(), 3, 'f', 0)
                            .arg(speedmap_bwd->getNextLimitDistance(), 6, 'f', 1);
        }
        else
        {
            if (epk[CAB2]->getEmergencyBrakeContact())
                DebugMsg += QString("EPK:EMERGENCY |");
            else
                DebugMsg += QString("EPK: WHISTLE  |");

            DebugMsg += QString("limit %1km/h|")
                            .arg(speedmap_bwd->getCurrentLimit(), 3, 'f', 0);
        }
        DebugMsg += QString("Code %1 (%2 Hz)| %3 (%4 m)")
                        .arg(coil_ALSN_bwd->getCode(), 1)
                        .arg(coil_ALSN_bwd->getFrequency(), 3, 'f', 0)
                        .arg(coil_ALSN_bwd->getNextSignalLiter())
                        .arg(coil_ALSN_bwd->getNextSignalDistance(), 6, 'f', 1);
    }
    else
    {
        if (epk[CAB2]->isKey())
        {
            DebugMsg += QString("EPK:OFF");
        }
        else
        {
            DebugMsg += QString("EPK:NO KEY");
        }
    }

    if (autopilot[CAB1] != nullptr && autopilot[CAB2] != nullptr)
    {
        QString auto_mode = QString("\n");

        if (km[CAB1]->isReversHandle())
        {
            auto_mode += autopilot[CAB1]->getDbgMsg();
        }

        if (km[CAB2]->isReversHandle())
        {
            auto_mode += autopilot[CAB2]->getDbgMsg();
        }

        DebugMsg += auto_mode;
    }
}
