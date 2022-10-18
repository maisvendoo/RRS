#include    "vl60.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::debugPrint(double t)
{
    DebugMsg = QString("t: %1 | %2 км | %3 км/ч ")
            .arg(t, 6, 'f', 1)
            .arg(railway_coord / 1000.0, 8, 'f', 3)
            .arg(velocity * Physics::kmh, 6, 'f', 1);
    DebugMsg += QString("| FwdQ:%1 | BwdQ:%2 ")
            .arg(QTMfwd, 8, 'f', 5)
            .arg(QTMbwd, 8, 'f', 5);
    DebugMsg += QString("| ТМ:%1 МПа | ЗР:%2 МПа | ТЦ:%3 МПа | Доп.Р.:%4")
            .arg(brakepipe->getPressure(), 5, 'f', 3)
            .arg(supply_reservoir->getPressure(), 5, 'f', 3)
            .arg(trolley_mech[0]->getBrakeCylinderPressure(), 5, 'f', 3)
            .arg(auxRate, 8, 'f', 5);
    DebugMsg += QString("| Vbp: %1   ")
            .arg(length * 0.035 * 0.035 * Physics::PI / 4.0, 8, 'f', 5);

    /*DebugMsg = QString("t: %1 x: %2 км v: %3 км/ч АЛСН: %4 Дист.: %5")
            .arg(t, 10, 'f', 2)
            .arg((railway_coord + dir * length / 2.0) / 1000.0, 8, 'f', 3)
            .arg(velocity * Physics::kmh, 6, 'f', 1)
            .arg(alsn_info.code_alsn, 2)
            .arg(alsn_info.signal_dist, 7, 'f', 1);*/
}
