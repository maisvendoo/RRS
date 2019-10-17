#include "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepPantographs(double t, double dt)
{
    // Управление разъединителями токоприемников
    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantoSwitcher[i]->setControl(keys);

        if (pantoSwitcher[i]->getState() == 3)
            pant_switch[i].set();

        if (pantoSwitcher[i]->getState() == 0)
            pant_switch[i].reset();

        if (pantoSwitcher[i]->getState() == 2 && pant_switch[i].getState())
            pantup_trigger[i].set();

        if (pantoSwitcher[i]->getState() == 1)
            pantup_trigger[i].reset();

        pantoSwitcher[i]->step(t, dt);

        // Подъем/опускание ТП
        pantographs[i]->setState(pant_switch[i].getState() && pantup_trigger[i].getState());

        pantographs[i]->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepBrakesMech(double t, double dt)
{
        brakesMech[0]->step(t, dt);
        brakesMech[1]->step(t, dt);

        Q_r[1] = brakesMech[0]->getBrakeTorque();
        Q_r[2] = brakesMech[0]->getBrakeTorque();
        Q_r[3] = brakesMech[0]->getBrakeTorque();

        Q_r[4] = brakesMech[1]->getBrakeTorque();
        Q_r[5] = brakesMech[1]->getBrakeTorque();
        Q_r[6] = brakesMech[1]->getBrakeTorque();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepFastSwitch(double t, double dt)
{
    bv->setHoldingCoilState(getHoldingCoilState());
    bv_return = getHoldingCoilState() && bv_return;
    bv->setReturn(bv_return);

    U_kr = max(pantographs[0]->getUout() * pant_switch[0].getState() ,
            pantographs[1]->getUout() * pant_switch[1].getState());

    bv->setU_in(U_kr);

    bv->setState(fast_switch_trigger.getState());
    bv->step(t, dt);

    if (fastSwitchSw->getState() == 3)
    {
        fast_switch_trigger.set();
        bv_return = true;
    }

    if (fastSwitchSw->getState() == 1)
    {
        fast_switch_trigger.reset();
        bv_return = false;
    }

    fastSwitchSw->setControl(keys);
    fastSwitchSw->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepProtection(double t, double dt)
{
    overload_relay->setCurrent(motor->getIa());
    overload_relay->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepTractionControl(double t, double dt)
{
    ip = 1.75;

    km21KR2->setHod(stepSwitch->getHod());
    km21KR2->setControl(keys);
    km21KR2->step(t, dt);

    stepSwitch->setCtrlState(km21KR2->getCtrlState());
    stepSwitch->setControl(keys);
    stepSwitch->step(t, dt);

    puskRez->setPoz(stepSwitch->getPoz());
    puskRez->step(t, dt);

    if (EDT)
        allowTrac.reset();
    if (stepSwitch->getPoz() == 0)
        allowTrac.set();

    motor->setDirection(km21KR2->getReverseState());
    motor->setBetaStep(km21KR2->getFieldStep());
    motor->setPoz(stepSwitch->getPoz());
    motor->setR(puskRez->getR());
    motor->setU(bv->getU_out() * stepSwitch->getSchemeState() * static_cast<double>(!EDT) * allowTrac.getState());
    motor->setOmega(wheel_omega[0] * ip);
    motor->setAmpermetersState(stepSwitch->getAmpermetersState());
    motor->step(t, dt);

    tracForce_kN = 0;

    for (size_t i = 0; i <= Q_a.size(); ++i)
    {
        Q_a[i] = (motor->getTorque() + generator->getTorque()) * ip;
        tracForce_kN += 2.0 * Q_a[i] / wheel_diameter / 1000.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepAirSupplySubsystem(double t, double dt)
{
    motor_compressor->setU(bv->getU_out() * static_cast<double>(mk_tumbler.getState()) * pressReg->getState());
    motor_compressor->setPressure(mainReservoir->getPressure());
    motor_compressor->step(t, dt);

    mainReservoir->setAirFlow(motor_compressor->getAirFlow());
    mainReservoir->step(t, dt);

    pressReg->setPressure(mainReservoir->getPressure());
    pressReg->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepBrakesControl(double t, double dt)
{
    brakeCrane->setChargePressure(charging_press);
    brakeCrane->setFeedLinePressure(mainReservoir->getPressure());
    brakeCrane->setBrakePipePressure(pTM);
    brakeCrane->setControl(keys);
    p0 = brakeCrane->getBrakePipeInitPressure();
    brakeCrane->step(t, dt);

    handleEDT->setPipiLinePressure(mainReservoir->getPressure());
    handleEDT->setBrefPressure(brakeRefRes->getPressure());
    handleEDT->setControl(keys);
    handleEDT->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepBrakesEquipment(double t, double dt)
{
    dako->setPgr(mainReservoir->getPressure());
    dako->setPtc(zpk->getPressure1());
    dako->setQvr(airSplit->getQ_out1() * static_cast<double>(!allowEDT));
    dako->setU(velocity);
    dako->setPkvt(zpk->getPressure2());
    //dako->setQ1(airSplit->getQ_out1());

    locoCrane->setFeedlinePressure(mainReservoir->getPressure());
    locoCrane->setBrakeCylinderPressure(zpk->getPressure2());
    locoCrane->setControl(keys);

    zpk->setInputFlow1(dako->getQtc());
    zpk->setInputFlow2(locoCrane->getBrakeCylinderFlow());
    zpk->setOutputPressure(pnSplit->getP_in());

    brakesMech[0]->setAirFlow(pnSplit->getQ_out1());

    airDistr->setBrakepipePressure(pTM);
    airDistr->setAirSupplyPressure(spareReservoir->getPressure());
    airDistr->setBrakeCylinderPressure(airSplit->getP_in());

    spareReservoir->setAirFlow(airDistr->getAirSupplyFlow());

    rd304->setBrakeCylPressure(brakesMech[1]->getBrakeCylinderPressure());
    rd304->setPipelinePressure(mainReservoir->getPressure());
    rd304->setWorkAirFlow(pnSplit->getQ_out2());

    brakesMech[1]->setAirFlow(rd304->getBrakeCylAirFlow());

    pnSplit->setP_out2(rd304->getWorkPressure());
    pnSplit->setP_out1(brakesMech[0]->getBrakeCylinderPressure());
    pnSplit->setQ_in(zpk->getOutputFlow());

    airSplit->setQ_in(airDistr->getBrakeCylinderAirFlow());
    airSplit->setP_out1(dako->getPy() * static_cast<double>(!allowEDT));
    airSplit->setP_out2(brakeRefRes->getPressure());

    brakeRefRes->setAirFlow(airSplit->getQ_out2() + handleEDT->getQ_bref());

    dako->step(t, dt);
    locoCrane->step(t, dt);
    zpk->step(t, dt);
    airDistr->step(t, dt);
    spareReservoir->step(t, dt);
    pnSplit->step(t, dt);
    rd304->step(t, dt);
    brakeRefRes->step(t, dt);
    airSplit->step(t, dt);
}

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
void CHS2T::stepSupportEquipment(double t, double dt)
{
    double R = 0.59;
    bool hod = stepSwitch->getHod();

    relValve->step(t, dt);

    motor_fan->setU(R * (motor->getIa() * !hod + abs(generator->getIa())));
    motor_fan->step(t, dt);

    blinds->setState((!hod && !stepSwitch->isZero()) || EDT);
    blinds->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CHS2T::getHoldingCoilState() const
{
    bool no_overload = (!static_cast<bool>(overload_relay->getState()));

    bool state = no_overload;

    return state;
}
