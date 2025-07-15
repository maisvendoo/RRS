#include    "vl60pk.h"

#include "ALSN-coil.h"
#include "ALSN-decoder.h"
#include "alsn-ukbm.h"
#include "automatic-train-stop.h"
#include "reservoir.h"
#include "sl2m.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepSafetyDevices(double t, double dt)
{
    // Приёмные катушки АЛСН
    coil_ALSN_fwd->step(t, dt);
    coil_ALSN_bwd->step(t, dt);

    // Дешифратор АЛСН
    alsn_decoder[CAB1]->setCoilSignal(coil_ALSN_fwd->getCode());
    alsn_decoder[CAB1]->step(t, dt);

    alsn_decoder[CAB2]->setCoilSignal(coil_ALSN_bwd->getCode());
    alsn_decoder[CAB2]->step(t, dt);

    // Скоростемер
    speed_meter->setOmega(wheel_omega[TED1]);
    speed_meter->step(t, dt);

    for (size_t cab_idx : {CAB1, CAB2})
    {
        // УКБМ
        safety_device[cab_idx]->setAlsnCode(alsn_decoder[cab_idx]->getCode());
        safety_device[cab_idx]->setRBstate(rb[cab_idx][RB_1].getState());
        safety_device[cab_idx]->setRBSstate(rb[cab_idx][RBS].getState());
        safety_device[cab_idx]->setKeyEPK(epk[cab_idx]->isKeyOn());
        safety_device[cab_idx]->setVelocity(speed_meter->getVelocity());
        safety_device[cab_idx]->step(t, dt);

        // Электропневматический клапан автостопа
        epk[cab_idx]->setFLpressure(main_reservoir->getPressure());
        epk[cab_idx]->setBPpressure(brakepipe->getPressure());
        epk[cab_idx]->setPowered(safety_device[cab_idx]->getEPKstate());
        epk[cab_idx]->setKeyOn(key_epk[cab_idx].getState());
        epk[cab_idx]->step(t, dt);
    }
}
