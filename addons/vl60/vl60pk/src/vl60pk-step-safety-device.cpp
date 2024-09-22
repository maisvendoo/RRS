#include    "vl60pk.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepSafetyDevices(double t, double dt)
{
    coil_ALSN_fwd->step(t, dt);
    coil_ALSN_bwd->step(t, dt);

    analogSignal[LS_W] = 0.0f;
    analogSignal[LS_YK] = 0.0f;
    analogSignal[LS_R] = 0.0f;
    analogSignal[LS_Y] = 0.0f;
    analogSignal[LS_G] = 0.0f;

    switch (coil_ALSN_fwd->getCode())
    {
    case ALSN::NO_CODE:

        analogSignal[LS_W] = 1.0f;

        break;

    case ALSN::RED_YELLOW:

        analogSignal[LS_YK] = 1.0f;

        break;

    case ALSN::YELLOW:

        analogSignal[LS_Y] = 1.0f;

        break;

    case ALSN::GREEN:

        analogSignal[LS_G] = 1.0f;

        break;
    }
}
