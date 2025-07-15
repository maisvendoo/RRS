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

    // УКБМ
    safety_device[CAB1]->setAlsnCode(alsn_decoder[CAB1]->getCode());
    safety_device[CAB1]->setRBstate(rb[CAB1][RB_1].getState());
    safety_device[CAB1]->setRBSstate(rb[CAB1][RBS].getState());
    safety_device[CAB1]->setKeyEPK(epk[CAB1]->isKeyOn());
    safety_device[CAB1]->setVelocity(speed_meter->getVelocity());
    safety_device[CAB1]->step(t, dt);

    safety_device[CAB2]->setAlsnCode(alsn_decoder[CAB2]->getCode());
    safety_device[CAB2]->setRBstate(rb[CAB2][RB_1].getState());
    safety_device[CAB2]->setRBSstate(rb[CAB2][RBS].getState());
    safety_device[CAB2]->setKeyEPK(epk[CAB2]->isKeyOn());
    safety_device[CAB2]->setVelocity(speed_meter->getVelocity());
    safety_device[CAB2]->step(t, dt);

    for (size_t cab_idx : {CAB1, CAB2})
    {
        // Электропневматический клапан автостопа
        epk[cab_idx]->setFLpressure(main_reservoir->getPressure());
        epk[cab_idx]->setBPpressure(brakepipe->getPressure());
        epk[cab_idx]->setPowered(safety_device[cab_idx]->getEPKstate());
        epk[cab_idx]->setKeyOn(key_epk[cab_idx].getState());
        epk[cab_idx]->step(t, dt);
    }
}
