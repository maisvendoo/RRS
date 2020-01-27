#include    "ep20.h"

void EP20::stepSignals()
{
    analogSignal[STRELKA_pTM] = static_cast<float>(pTM / 1.0);
    analogSignal[STRELKA_pGR] = static_cast<float>(main_reservoir->getPressure() / 1.6);
    analogSignal[STRELKA_pTC] = static_cast<float>(brake_mech[FWD_TROLLEY]->getBrakeCylinderPressure() / 1.6);
    analogSignal[STRELKA_pUR] = static_cast<float>(krm->getEqReservoirPressure() / 1.0);

    analogSignal[RUK_KRM130] = krm->getHandlePosition();
    analogSignal[RUK_KVT224] = static_cast<float>(kvt->getHandlePosition());

    analogSignal[KMB2_Real] = kmb2->getTractionPosition();
    analogSignal[KMB2_Fake] = kmb2->getVelocityPosition();

    analogSignal[KeyCard_Fake] = kmb2->getPovorot();
    analogSignal[KeyCard_Low] = kmb2->getS3();

    analogSignal[BLOK_TEST] = static_cast<float>(brake_mech[FWD_TROLLEY]->getBrakeCylinderPressure());

}
