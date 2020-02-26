#include    "vl60.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::debugPrint(double t)
{
    DebugMsg = QString("t: %1 x: %2 км v: %3 км/ч АЛСН: %4 Дист.: %5")

            .arg(t, 10, 'f', 2)
            .arg((railway_coord + dir * length / 2.0) / 1000.0, 8, 'f', 3)
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(alsn_info.code_alsn, 2)
            .arg(alsn_info.signal_dist, 7, 'f', 1);

    //DebugMsg = brake_crane->getDebugMsg();
}
