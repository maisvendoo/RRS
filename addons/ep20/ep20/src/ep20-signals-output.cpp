#include    "ep20.h"

void EP20::stepSignals()
{
    analogSignal[STRELKA_pTM] = static_cast<float>(pTM / 1.0);
    analogSignal[STRELKA_pGR] = static_cast<float>(main_reservoir->getPressure() / 1.6);
    analogSignal[STRELKA_pTC] = static_cast<float>(zpk->getPressure1() / 1.6);
    analogSignal[STRELKA_pUR] = static_cast<float>(krm->getEqReservoirPressure() / 1.0);
}
