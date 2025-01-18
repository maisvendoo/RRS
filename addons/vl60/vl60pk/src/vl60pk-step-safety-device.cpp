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
    alsn_decoder->setCoilSignal(coil_ALSN_fwd->getCode());
    alsn_decoder->step(t, dt);

    // Скоростемер
    speed_meter->setOmega(wheel_omega[TED1]);
    speed_meter->step(t, dt);

    // УКБМ
    safety_device->setAlsnCode(alsn_decoder->getCode());
    safety_device->setRBstate(rb[RB_1].getState());
    safety_device->setRBSstate(rb[RBS].getState());
    safety_device->setKeyEPK(epk->isKeyOn());
    safety_device->setVelocity(speed_meter->getVelocity());
    safety_device->step(t, dt);

    // Электропневматический клапан автостопа
    epk->setFLpressure(main_reservoir->getPressure());
    epk->setBPpressure(brakepipe->getPressure());
    epk->setPowered(safety_device->getEPKstate());
    epk->setKeyOn(key_epk.getState());
    epk->step(t, dt);
}
