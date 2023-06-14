#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepEDT(double t, double dt)
{
    pulseConv->setUakb(110.0 * static_cast<double>(EDT));
    pulseConv->setU(BrakeReg->getU());
    pulseConv->setUt(generator->getUt() * static_cast<double>(EDT));

    generator->setUf(pulseConv->getUf());
    generator->setOmega(wheel_omega[0] * ip);
    generator->setRt(3.35);

    BrakeReg->setActive(EDTSwitch.getState());
    BrakeReg->setAllowEDT(dako->isEDTAllow());
    BrakeReg->setIa(generator->getIa());
    BrakeReg->setIf(generator->getIf());
    BrakeReg->setBref(brake_ref_res->getPressure());

    pulseConv->step(t, dt);
    generator->step(t, dt);
    BrakeReg->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepEDT2(double t, double dt)
{
    if (EDTSwitch.getState())
    {
        if ( (brake_ref_res->getPressure() >= 0.07) && !EDT_timer.isStarted())
        {
            dropPosition = true;
            EDT_timer.start();
        }

        if (allowEDT)
        {
            if (!dako->isEDTAllow() || (brake_ref_res->getPressure() < 0.07) )
            {
                disableEDT();
                dropPosition = false;
                BrakeReg->reset();
            }
        }
    }
    EDT_timer.step(t, dt);
}
