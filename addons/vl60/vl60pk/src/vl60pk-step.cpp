#include    "vl60pk.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::slotAutoStart()
{
    if (start_count < triggers.size())
    {
        triggers[start_count]->set();

        if (!pantographs[0]->isUp() && !pantographs[1]->isUp() &&
                (triggers[start_count] == &gv_tumbler))
            return;

        if (main_switch->getState())
            gv_return_tumbler.reset();

        start_count++;
    }
    else
    {
        autoStartTimer->stop();
        controller->setReversPos(REVERS_FORWARD);
        start_count = 0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepPantographsControl(double t, double dt)
{
    pantographs[0]->setState(pant1_tumbler.getState() && pants_tumbler.getState());
    pantographs[1]->setState(pant2_tumbler.getState() && pants_tumbler.getState());

    for (auto pant : pantographs)
    {
        // Задаем текущее напряжение КС (пока что через константу)
        pant->setUks(Uks);
        // Моделируем работу токоприемников
        pant->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepMainSwitchControl(double t, double dt)
{
    // Подаем на вход напряжение с крышевой шины, на которую включены
    // оба токоприемника
    main_switch->setU_in(max(pantographs[0]->getUout(), pantographs[1]->getUout()));

    // Задаем состояние органов управления ГВ
    main_switch->setState(gv_tumbler.getState());
    main_switch->setReturn(gv_return_tumbler.getState());

    // Подаем питание на удерживающую катушку ГВ
    main_switch->setHoldingCoilState(getHoldingCoilState());

    gauge_KV_ks->setInput(main_switch->getU_out() / 30000.0);

    // Моделируем работу ГВ
    main_switch->step(t, dt);

    gauge_KV_ks->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepTracTransformer(double t, double dt)
{
    // Задаем напряжение на первичной обмотке (с выхода ГВ)
    trac_trans->setU1(main_switch->getU_out());
    trac_trans->setPosition(main_controller->getPosition());

    trac_trans->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepPhaseSplitter(double t, double dt)
{
    double U_power = trac_trans->getU_sn() * static_cast<double>(fr_tumbler.getState());
    phase_spliter->setU_power(U_power);

    phase_spliter->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepMotorFans(double t, double dt)
{
    for (size_t i = 0; i < NUM_MOTOR_FANS; ++i)
    {
        ACMotorFan *mf = motor_fans[i];
        mf->setPowerVoltage(phase_spliter->getU_out() * static_cast<double>(mv_tumblers[i].getState()));
        mf->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepTractionControl(double t, double dt)
{
    controller->setControl(keys);
    controller->step(t, dt);

    main_controller->enable(cu_tumbler.getState() && brake_lock->isUnlocked());
    main_controller->setKMstate(controller->getState());
    main_controller->step(t, dt);

    gauge_KV_motors->setInput(vu[VU1]->getU_out());
    gauge_KV_motors->step(t, dt);

    motor[TED1]->setU(vu[VU1]->getU_out() * static_cast<double>(line_contactor[TED1].getState()));
    motor[TED2]->setU(vu[VU1]->getU_out() * static_cast<double>(line_contactor[TED2].getState()));
    motor[TED3]->setU(vu[VU1]->getU_out() * static_cast<double>(line_contactor[TED3].getState()));

    motor[TED4]->setU(vu[VU2]->getU_out() * static_cast<double>(line_contactor[TED4].getState()));
    motor[TED5]->setU(vu[VU2]->getU_out() * static_cast<double>(line_contactor[TED5].getState()));
    motor[TED6]->setU(vu[VU2]->getU_out() * static_cast<double>(line_contactor[TED6].getState()));

    km_state_t km_state = controller->getState();

    double I_vu = 0;

    for (size_t i = 0; i < motor.size(); ++i)
    {
        motor[i]->setDirection(km_state.revers_ref_state);
        motor[i]->setOmega(ip * wheel_omega[i]);
        motor[i]->setBetaStep(km_state.field_loosen_pos);
        motor[i]->step(t, dt);
        Q_a[i+1] = motor[i]->getTorque() * ip;

        I_vu += motor[i]->getIa();

        overload_relay[i]->setCurrent(motor[i]->getIa());
    }

    for (auto v : vu)
    {
        v->setI_out(I_vu);
        v->setU_in(trac_trans->getTracVoltage());
        v->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepLineContactors(double t, double dt)
{
    (void) t;
    (void) dt;

    km_state_t km_state = controller->getState();

    bool motor_fans_state = true;

    for (auto mf: motor_fans)
    {
        motor_fans_state &= mf->isReady();
    }

    bool lk_state = !km_state.pos_state[POS_BV] &&
                    !km_state.pos_state[POS_ZERO] &&
                    motor_fans_state && main_controller->isReady();

    lineContactorsControl(lk_state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::lineContactorsControl(bool state)
{
    for (size_t i = 0; i < line_contactor.size(); ++i)
    {
        if (state)
            line_contactor[i].set();
        else
            line_contactor[i].reset();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float VL60pk::isLineContactorsOff()
{
    bool state = true;

    for (size_t i = 0; i < line_contactor.size(); ++i)
    {
        state = state && line_contactor[i].getState();
    }

    return 1.0f - static_cast<float>(state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepOtherEquipment(double t, double dt)
{
    speed_meter->setOmega(wheel_omega[TED1]);
    speed_meter->step(t, dt);

    horn->setFLpressure(main_reservoir->getPressure());
    horn->setControl(keys);
    horn->step(t, dt);

    // Система подачи песка
    sand_system->setFLpressure(main_reservoir->getPressure());
    sand_system->setControl(keys);
    sand_system->step(t, dt);
    for (size_t i = 0; i < num_axis; ++i)
    {
        // Пересчёт трения колесо-рельс
        psi[i] = sand_system->getWheelRailFrictionCoeff(psi[i]);
    }
    // Пересчёт массы локомотива
    payload_coeff = sand_system->getSandLevel();
    setPayloadCoeff(payload_coeff);

    if (reg == nullptr)
        return;
    reg->print(QString("%1;%2;%3")
                   .arg(t,8,'f',3)
                   .arg(coupling_fwd->getCurrentForce(),13,'f',3)
                   .arg(coupling_bwd->getCurrentForce(),13,'f',3));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepTapSound()
{
    double speed = abs(this->velocity) * 3.6;

    for (int i = 0; i < tap_sounds.count(); ++i)
    {
        emit volumeCurveStep(tap_sounds[i], static_cast<float>(speed));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VL60pk::getTractionForce()
{
    double sum_force = 0.0;

    for (size_t i = 0; i < motor.size(); ++i)
    {
        sum_force += motor[i]->getTorque() * ip * 2.0 / wheel_diameter[i];
    }

    return sum_force;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VL60pk::getHoldingCoilState() const
{
    km_state_t km_state = controller->getState();

    bool overload = false;

    for (auto ov_relay : overload_relay)
    {
        overload |= ov_relay->getState();
    }

    bool state = !km_state.pos_state[POS_BV] && (!overload);

    return state;
}
