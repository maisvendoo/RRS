#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(dt)

    DebugMsg = QString("t: %1 | %2 км | %3 км/ч ")
            .arg(t, 6, 'f', 1)
            .arg(railway_coord / 1000.0, 8, 'f', 3)
            .arg(velocity * Physics::kmh, 6, 'f', 1);
    DebugMsg += QString("| FwdQ:%1 | BwdQ:%2 ")
            .arg(QTMfwd, 8, 'f', 5)
            .arg(QTMbwd, 8, 'f', 5);
    DebugMsg += QString("| ТМ:%1 МПа | ЗР:%2 МПа | ТЦ:%3 МПа | Доп.Р.:%4")
            .arg(brakepipe->getPressure(), 5, 'f', 3)
            .arg(spareReservoir->getPressure(), 5, 'f', 3)
            .arg(brakesMech[1]->getBrakeCylinderPressure(), 5, 'f', 3)
            .arg(auxRate, 8, 'f', 5);
    DebugMsg += QString("| Vbp: %1   ")
            .arg(length * 0.035 * 0.035 * Physics::PI / 4.0, 8, 'f', 5);

    /*DebugMsg = QString("t: %1 x: %2 v: %3 ТМ: %4 ЗР: %5 ТЦ: %6 Uэпт: %7 Iэпт: %8")
        .arg(t, 10, 'f', 1)
        .arg(railway_coord / 1000.0, 5, 'f', 1)
        .arg(velocity * Physics::kmh, 5, 'f', 1)
        .arg(pTM, 5, 'f', 2)
        .arg(spareReservoir->getPressure(), 5, 'f', 2)
        .arg(brakesMech[1]->getBrakeCylinderPressure(), 5, 'f', 2)
        .arg(ept_converter->getU_out(), 5, 'f', 2)
        .arg(ept_current[0], 6, 'f', 2);*/

    /*DebugMsg = QString("t: %1 x: %2 Время. график: %3 Дист. сигн.: %4" )
                       .arg(t, 10, 'f', 1)
                       .arg(railway_coord / 1000.0, 5, 'f', 1)
                       .arg(alsn_info.current_time, 8)
                       .arg(alsn_info.signal_dist, 4, 'f', 0);*/
}
