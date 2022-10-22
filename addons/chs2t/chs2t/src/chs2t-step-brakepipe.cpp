#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepBrakepipe(double t, double dt)
{
    // Подключение потоков из межвагонных содинений и концевых кранов в рукава
    hose_tm_fwd->setAirFlow(QTMfwd + anglecock_tm_fwd->getQ_hose());
    hose_tm_fwd->step(t, dt);
    pTMfwd = hose_tm_fwd->getPressure();
    hose_tm_bwd->setAirFlow(QTMfwd + anglecock_tm_bwd->getQ_hose());
    hose_tm_bwd->step(t, dt);
    pTMbwd = hose_tm_bwd->getPressure();

    // Потоки через концевые краны
    anglecock_tm_fwd->setP_hose(hose_tm_fwd->getPressure());
    anglecock_tm_fwd->setP_pipe(brakepipe->getPressure());
    anglecock_tm_fwd->step(t, dt);
    anglecock_tm_bwd->setP_hose(hose_tm_bwd->getPressure());
    anglecock_tm_bwd->setP_pipe(brakepipe->getPressure());
    anglecock_tm_bwd->step(t, dt);

    // Подключение потоков из оборудования в ТМ
    double QTM = anglecock_tm_fwd->getQ_pipe() + anglecock_tm_bwd->getQ_pipe();
    QTM += -1.0 * (airDistr->getAuxRate());
    QTM += -1.0 * (autoTrainStop->getEmergencyBrakeRate());
    QTM += 1.0 * (brakeCrane->getBrakePipeInitPressure() - brakepipe->getPressure());
    brakepipe->setAirFlow(QTM);
    brakepipe->step(t, dt);
}
