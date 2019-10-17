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

    BrakeReg->setAllowEDT(dako->isEDTAllow());
    BrakeReg->setIa(generator->getIa());
    BrakeReg->setIf(generator->getIf());
    BrakeReg->setBref(brakeRefRes->getPressure());

    allowEDT = EDTSwitch.getState() && dako->isEDTAllow();

    EDT = static_cast<bool>(hs_p(brakeRefRes->getPressure() - 0.07));

    /*if (!dako->isEDTAllow())
        EDTValve.reset();*/

    pulseConv->step(t, dt);
    generator->step(t, dt);
    BrakeReg->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepEDT2(double t, double dt)
{
    if (EDT)
    {
        if (brakeRefRes->getPressure() >= 0.07)
        {
            dropPosition = true;
            timer.start();
//            if (timer.)
        }
    }
}
