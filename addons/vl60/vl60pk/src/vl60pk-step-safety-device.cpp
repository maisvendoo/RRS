#include    "vl60pk.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepSafetyDevices(double t, double dt)
{
    coil_ALSN_fwd->step(t, dt);
    coil_ALSN_bwd->step(t, dt);

    analogSignal[LS_W] = safety_device->getWhiteLamp();
    analogSignal[LS_YK] = safety_device->getRedYellowLamp();
    analogSignal[LS_R] = safety_device->getRedLamp();
    analogSignal[LS_Y] = safety_device->getYellowLamp();
    analogSignal[LS_G] = safety_device->getGreenLamp();


    epk->setFLpressure(main_reservoir->getPressure());
    epk->setBPpressure(brakepipe->getPressure());
    epk->setPowered(safety_device->getEPKstate());
    epk->setKeyOn(key_epk.getState());
    epk->step(t, dt);

    safety_device->setAlsnCode(coil_ALSN_fwd->getCode());
    safety_device->setRBstate(rb[RB_1].getState());
    safety_device->setRBSstate(rb[RBS].getState());
    safety_device->setKeyEPK(epk->isKeyOn());
    safety_device->setVelocity(speed_meter->getVelocity());
    safety_device->step(t, dt);
}
