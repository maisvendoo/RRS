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
        if (!bv_return)
            emit soundPlay("BV-on");
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
    km21KR2->setControl(keys, control_signals);
    km21KR2->step(t, dt);

    stepSwitch->setDropPosition(dropPosition);
    stepSwitch->setCtrlState(km21KR2->getCtrlState());
    stepSwitch->setControl(keys);
    stepSwitch->step(t, dt);

    puskRez->setPoz(stepSwitch->getPoz());
    puskRez->step(t, dt);

    if (EDT || (!epk->getStateKey()) || (!emergency_valve->isTractionAllow()))
    {
        allowTrac.reset();
    }
    else
    {
        if (stepSwitch->getPoz() == 0)
            allowTrac.set();
    }

    motor->setDirection(stepSwitch->getReverseState());
    motor->setBetaStep(stepSwitch->getFieldStep());
    motor->setPoz(stepSwitch->getPoz());
    motor->setR(puskRez->getR());
    motor->setU(bv->getU_out() * stepSwitch->getSchemeState() * static_cast<double>(allowTrac.getState()));
    motor->setOmega(wheel_omega[0] * ip);
    motor->setAmpermetersState(stepSwitch->getAmpermetersState());
    motor->step(t, dt);

    tracForce_kN = 0;

    for (size_t i = 1; i < Q_a.size(); ++i)
    {
        Q_a[i] = (motor->getTorque() + generator->getTorque()) * ip;
        tracForce_kN += 2.0 * Q_a[i] / wheel_diameter[i - 1] / 1000.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepSupportEquipment(double t, double dt)
{
    double R = 0.6;
    bool hod = stepSwitch->getHod();

    // Мотор-вентилятор ПТР
    motor_fan_ptr->setU(R * (motor->getIa() * !hod + abs(generator->getIa())));
    motor_fan_ptr->step(t, dt);

    motor_fan_switcher->setControl(keys);

    if (motor_fan_switcher->getState() == 0)
    {
        motor_fan[0]->setU(0.0);
        motor_fan[1]->setU(0.0);
    }

    if (motor_fan_switcher->getState() == 1)
    {
        motor_fan[0]->setU((bv->getU_out() / 2.0) * (stepSwitch->getPoz() > 0 || motor_fan[0]->getU() > 0));
        motor_fan[1]->setU((bv->getU_out() / 2.0) * (stepSwitch->getPoz() > 0 || motor_fan[1]->getU() > 0));
    }

    if (motor_fan_switcher->getState() == 2)
    {
        motor_fan[0]->setU(bv->getU_out() / 2.0);
        motor_fan[1]->setU(bv->getU_out() / 2.0);
    }

    motor_fan_switcher->step(t, dt);
    motor_fan[0]->step(t, dt);
    motor_fan[1]->step(t, dt);

    blindsSwitcher->setControl(keys);

    if (blindsSwitcher->getState() == 0 || blindsSwitcher->getState() == 1)
    {
        blinds->setState(false);
    }

    if (blindsSwitcher->getState() == 2)
    {
        blinds->setState(true);
    }

    if (blindsSwitcher->getState() == 3 || blindsSwitcher->getState() == 4)
    {
        blinds->setState((!hod && !stepSwitch->isZero()) || EDT);
    }

    blindsSwitcher->step(t, dt);
    blinds->step(t, dt);

    energy_counter->setFullPower(Uks * (motor->getI12() + motor->getI34() + motor->getI56()) );
    energy_counter->setResistorsPower( puskRez->getR() * ( pow(motor->getI12(), 2) + pow(motor->getI34(), 2) + pow(motor->getI56(), 2) ) );
    energy_counter->step(t, dt);

    safety_device->setRBstate(state_RB);
    safety_device->setRBSstate(state_RBS);
    safety_device->setAlsnCode(alsn_info.code_alsn);
    safety_device->setVelocity(velocity);
    safety_device->setKeyEPK(epk->getStateKey());
    safety_device->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CHS2T::getHoldingCoilState() const
{
    bool no_overload = (!static_cast<bool>(overload_relay->getState()));

    return no_overload;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepTapSound()
{
    double speed = abs(this->velocity) * 3.6;

    for (int i = 0; i < tap_sounds.count(); ++i)
    {
        emit volumeCurveStep(tap_sounds[i], static_cast<float>(speed));
    }
}
