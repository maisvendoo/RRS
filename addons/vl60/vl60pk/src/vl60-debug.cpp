#include    "vl60.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::debugPrint(double t)
{
    DebugMsg = QString("t: %1 x: %12 км v: %11 км/ч ЗР: %2 МПа ТЦ1: %3 ТЦ2: %4 Наж. на колодку: %5 кН Uву: %10 В Uтэд: %6 В Поз.: %7 Iя: %8 А Iв: %9 А")

            .arg(t, 10, 'f', 2)
            .arg(supply_reservoir->getPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_FWD]->getBrakeCylinderPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_BWD]->getBrakeCylinderPressure(), 4, 'f', 2)
            .arg(trolley_mech[TROLLEY_FWD]->getShoeForce() / 1000.0, 5, 'f', 1)
            .arg(motor[TED1]->getUd(), 6, 'f', 1)
            .arg(trac_trans->getPosName(), 2)
            .arg(motor[TED1]->getIa(), 6,'f',1)
            .arg(motor[TED1]->getIf(), 6,'f',1)
            .arg(vu[VU1]->getU_out(), 6, 'f', 1)
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(railway_coord / 1000.0, 7, 'f', 2);

    //DebugMsg = brake_crane->getDebugMsg();
}
