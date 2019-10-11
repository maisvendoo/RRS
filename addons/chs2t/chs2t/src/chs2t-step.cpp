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
void CHS2T::stepDebugMsg(double t, double dt)
{
    Q_UNUSED(dt)

    DebugMsg = QString("t = %1")
        .arg(t, 10, 'f', 1);
}

<<<<<<< HEAD
=======
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepSignals()
{
    analogSignal[STRELKA_POS] = static_cast<float>(stepSwitch->getPoz()) / 42.0f;

    analogSignal[STRELKA_AMP1] = static_cast<float>(motor->getI12() / 1000.0);

    if (EDT)
    {
        analogSignal[STRELKA_AMP3] = static_cast<float>(abs(generator->getIa()) / 1000.0);
        analogSignal[STRELKA_AMP2] = static_cast<float>(abs(generator->getIf()) / 1000.0);
    }
    else
    {
        analogSignal[STRELKA_AMP3] = static_cast<float>(motor->getI56() / 1000.0);
        analogSignal[STRELKA_AMP2] = static_cast<float>(motor->getI34() / 1000.0);
    }

    analogSignal[STRELKA_PM] = static_cast<float>(mainReservoir->getPressure() / 1.6);
    analogSignal[STRELKA_TC] = static_cast<float>(brakesMech[0]->getBrakeCylinderPressure() / 1.0);
    analogSignal[STRELKA_EDT] = static_cast<float>(brakeRefRes->getPressure() / 1.0);
    analogSignal[STRELKA_UR] = static_cast<float>(brakeCrane->getEqReservoirPressure() / 1.0);
    analogSignal[STRELKA_TM] = static_cast<float>(pTM / 1.0);

    analogSignal[STRELKA_UKS] = static_cast<float>(U_kr / 4000.0);

    analogSignal[KRAN395_RUK] = static_cast<float>(brakeCrane->getHandlePosition());
    analogSignal[KRAN254_RUK] = static_cast<float>(locoCrane->getHandlePosition());

    analogSignal[KONTROLLER] = static_cast<float>(km21KR2->getMainShaftPos());
    analogSignal[SHTURVAL] = static_cast<float>(km21KR2->getMainShaftHeight());

    analogSignal[SHTUR_SVISTOK] = static_cast<float>(horn->isSvistok());

    analogSignal[REVERSOR] = static_cast<float>(km21KR2->getReverseState());

    analogSignal[HANDLE_RT] = handleEDT->getHandlePos();

    analogSignal[SIGLIGHT_P] = static_cast<float>(stepSwitch->isParallel());
    analogSignal[SIGLIGHT_SP] = static_cast<float>(stepSwitch->isSeriesParallel());
    analogSignal[SIGLIGHT_S] = static_cast<float>(stepSwitch->isSeries());
    analogSignal[SIGLIGHT_ZERO] = static_cast<float>(stepSwitch->isZero());

    analogSignal[SIGLIGHT_R] = static_cast<float>(EDT);

    analogSignal[SIGLIGHT_NO_BRAKES_RELEASE] = static_cast<float>(brakesMech[0]->getBrakeCylinderPressure() >= 0.1);

    analogSignal[PANT1] = static_cast<float>(pantographs[0]->getHeight());
    analogSignal[PANT2] = static_cast<float>(pantographs[1]->getHeight());

    analogSignal[SW_PNT1] = pantoSwitcher[0]->getHandlePos();
    analogSignal[SW_PNT2] = pantoSwitcher[1]->getHandlePos();

    analogSignal[SIGLIGHT_RAZED] = static_cast<float>( !pant_switch[0].getState() && !pant_switch[1].getState() );

    analogSignal[INDICATOR_BV] = static_cast<float>(bv->getLampState());

    analogSignal[SW_BV] = fastSwitchSw->getHandlePos();

    analogSignal[WHEEL_1] = static_cast<float>(dir * wheel_rotation_angle[0] / 2.0 / Physics::PI);
    analogSignal[WHEEL_2] = static_cast<float>(dir * wheel_rotation_angle[1] / 2.0 / Physics::PI);
    analogSignal[WHEEL_3] = static_cast<float>(dir * wheel_rotation_angle[2] / 2.0 / Physics::PI);
    analogSignal[WHEEL_4] = static_cast<float>(dir * wheel_rotation_angle[3] / 2.0 / Physics::PI);
    analogSignal[WHEEL_5] = static_cast<float>(dir * wheel_rotation_angle[4] / 2.0 / Physics::PI);
    analogSignal[WHEEL_6] = static_cast<float>(dir * wheel_rotation_angle[5] / 2.0 / Physics::PI);
}

>>>>>>> a379395ffd05141a7291501af5800c2a5e5093db
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::registrate(double t, double dt)
{
    QString msg = QString("%1 %2 %3 %4")
            .arg(t)
            .arg(velocity * Physics::kmh)
            .arg(tracForce_kN)
            .arg(motor->getIa());

    reg->print(msg, t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CHS2T::getHoldingCoilState() const
{
    bool no_overload = true;

    no_overload = no_overload && (!static_cast<bool>(overload_relay->getState()));

    bool state = no_overload;

    return state;
}
