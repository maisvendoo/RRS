#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepBrakepipe(double t, double dt)
{
    // Подключение ТМ к рукавам через концевые краны
    anglecock_tm_fwd->setQ_hose(QTMfwd);
    anglecock_tm_fwd->setP_pipe(brakepipe->getPressure());
    anglecock_tm_fwd->step(t, dt);
    anglecock_tm_bwd->setQ_hose(QTMbwd);
    anglecock_tm_bwd->setP_pipe(brakepipe->getPressure());
    anglecock_tm_bwd->step(t, dt);

    // Давление в рукавах
    pTMfwd = anglecock_tm_fwd->getP_hose();
    pTMbwd = anglecock_tm_bwd->getP_hose();

    // Подключение потоков из оборудования в ТМ
    double QTM = anglecock_tm_fwd->getQ_pipe() + anglecock_tm_bwd->getQ_pipe();
    QTM += -1.0 * (airDistr->getAuxRate());
    QTM += -1.0 * (autoTrainStop->getEmergencyBrakeRate());
    QTM += 1.0 * (brakeCrane->getBrakePipeInitPressure() - brakepipe->getPressure());
    brakepipe->setAirFlow(QTM);
    brakepipe->step(t, dt);
}
