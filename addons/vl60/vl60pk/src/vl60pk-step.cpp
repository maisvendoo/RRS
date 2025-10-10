#include    <vl60pk.h>
#include    <Journal.h>

#include "automatic-train-stop.h"
#include "dc-motor.h"
#include "ekg-8g.h"
#include "kme-60-044.h"
#include "motor-fan-ac.h"
#include "oscillator.h"
#include "overload-relay.h"
#include "pantograph.h"
#include "phase-splitter.h"
#include "pneumo-brake-lock.h"
#include "protective-device.h"
#include "rectifier.h"
#include "registrator.h"
#include "relay.h"
#include "reservoir.h"
#include "sanding-system.h"
#include "trac-transformer.h"
#include "train-horn.h"
#include "spotlight.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::slotAutoStart()
{
    if (start_count < triggers.size())
    {
        triggers[start_count]->set();

        if (!pantographs[0]->isUp() && !pantographs[1]->isUp() &&
                (triggers[start_count] == &gv_tumbler[autostart_cab]))
            return;

        if (main_switch->getState())
            gv_return_tumbler[autostart_cab].reset();

        start_count++;
    }
    else
    {
        autoStartTimer->stop();
        controller[autostart_cab]->setReversHandlePos(REVERS_FORWARD);
        start_count = 0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepPantographsControl(const double& t, const double& dt)
{
    // Подъем переднего токоприемника
    bool is_PANT1_ON = (pants_tumbler[CAB1].getState() || pants_tumbler[CAB2].getState()) &&
                       (pant1_tumbler[CAB1].getState() || pant2_tumbler[CAB2].getState());

    // Подъем заднего токоприемника
    bool is_PANT2_ON = (pants_tumbler[CAB1].getState() || pants_tumbler[CAB2].getState()) &&
                       (pant1_tumbler[CAB2].getState() || pant2_tumbler[CAB1].getState());

    pantographs[0]->setState(is_PANT1_ON);
    pantographs[1]->setState(is_PANT2_ON);

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
void VL60pk::stepMainSwitchControl(const double& t, const double& dt)
{
    // Подаем на вход напряжение с крышевой шины, на которую включены
    // оба токоприемника
    main_switch->setU_in(max(pantographs[0]->getUout(), pantographs[1]->getUout()));

    // Задаем состояние органов управления ГВ
    main_switch->setState(gv_tumbler[CAB1].getState() || gv_tumbler[CAB2].getState());
    main_switch->setReturn(gv_return_tumbler[CAB1].getState() || gv_return_tumbler[CAB2].getState());

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
void VL60pk::stepTracTransformer(const double& t, const double& dt)
{
    // Задаем напряжение на первичной обмотке (с выхода ГВ)
    trac_trans->setU1(main_switch->getU_out());
    trac_trans->setPosition(main_controller->getPosition());

    trac_trans->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepPhaseSplitter(const double& t, const double& dt)
{
    bool is_FR_ON = fr_tumbler[CAB1].getState() || fr_tumbler[CAB2].getState();
    phase_spliter->setU_power(trac_trans->getU_sn() * static_cast<double>(is_FR_ON));

    phase_spliter->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepMotorFans(const double& t, const double& dt)
{
    bool is_MV_ON;
    for (size_t i = 0; i < NUM_MOTOR_FANS; ++i)
    {
        ACMotorFan *mf = motor_fans[i];
        is_MV_ON = mv_tumblers[CAB1][i].getState() || mv_tumblers[CAB2][i].getState();
        mf->setPowerVoltage(phase_spliter->getU_out() * static_cast<double>(is_MV_ON));
        mf->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepTractionControl(const double& t, const double& dt)
{
    for (size_t i : {CAB1, CAB2})
    {
        controller[i]->step(t, dt);
    }

    main_controller->enable((cu_tumbler[CAB1].getState() || cu_tumbler[CAB2].getState()) &&
                            (brake_lock[CAB1]->isStateOn() || brake_lock[CAB2]->isStateOn()));
    main_controller->setKMstate(controller[CAB1]->getState(), controller[CAB2]->getState());
    main_controller->step(t, dt);

    gauge_KV_motors->setInput(vu[VU1]->getU_out());
    gauge_KV_motors->step(t, dt);

    motor[TED1]->setU(vu[VU1]->getU_out() * static_cast<double>(linear_contactor[TED1]->getContactState(LC_TED)));
    motor[TED2]->setU(vu[VU1]->getU_out() * static_cast<double>(linear_contactor[TED2]->getContactState(LC_TED)));
    motor[TED3]->setU(vu[VU1]->getU_out() * static_cast<double>(linear_contactor[TED3]->getContactState(LC_TED)));

    motor[TED4]->setU(vu[VU2]->getU_out() * static_cast<double>(linear_contactor[TED4]->getContactState(LC_TED)));
    motor[TED5]->setU(vu[VU2]->getU_out() * static_cast<double>(linear_contactor[TED5]->getContactState(LC_TED)));
    motor[TED6]->setU(vu[VU2]->getU_out() * static_cast<double>(linear_contactor[TED6]->getContactState(LC_TED)));

    // Полярность включения ТЭД
    int motor_dir = std::clamp(controller[CAB1]->getState().revers_ref_state - controller[CAB2]->getState().revers_ref_state, -1, 1);
    // Ступень ослабления возбуждения
    int motor_field_loosen_pos = std::max(controller[CAB1]->getState().field_loosen_pos, controller[CAB2]->getState().field_loosen_pos);

    double I_vu = 0.0;

    for (size_t i = 0; i < motor.size(); ++i)
    {
        motor[i]->setDirection(motor_dir);
        motor[i]->setOmega(ip * wheel_omega[i]);
        motor[i]->setBetaStep(motor_field_loosen_pos);
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
void VL60pk::stepLineContactors(const double& t, const double& dt)
{
    (void) t;
    (void) dt;

    int revers_state = std::clamp(controller[CAB1]->getState().revers_ref_state - controller[CAB2]->getState().revers_ref_state, -1, 1);

    bool is_KM_ZERO = controller[CAB1]->getState().pos_state[POS_ZERO] && controller[CAB2]->getState().pos_state[POS_ZERO];

    bool motor_fans_state = true;

    for (auto mf: motor_fans)
    {
        motor_fans_state &= mf->isReady();
    }

    // Подготовка цепей линейных контакторов

    // Состояние провода Н6
    bool is_BP_released = brakepipe->getPressure() > 0.3;

    bool is_H6_ON = (cu_tumbler[CAB1].getState() || cu_tumbler[CAB2].getState())  &&
                    (epk[CAB1]->isKeyOn() || epk[CAB2]->isKeyOn()) &&
                    is_BP_released &&
                    (revers_state != 0) &&
                    (!is_KM_ZERO) &&
                    motor_fans_state;

    for (auto lc : linear_contactor)
    {
        bool is_LC_ON = is_H6_ON &&
                        (main_controller->isZeroPosition() || lc->getContactState(LC_SELF));

        lc->setVoltage(U_bat * static_cast<double>(is_LC_ON));
        lc->step(t, dt);
    }
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

    for (size_t i = 0; i < linear_contactor.size(); ++i)
    {
        state = state && linear_contactor[i]->getContactState(LC_TED_LAMP);
    }

    return static_cast<float>(state);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepOtherEquipment(const double& t, const double& dt)
{
    horn[CAB1]->setFLpressure(main_reservoir->getPressure());
    horn[CAB1]->step(t, dt);
    horn[CAB2]->setFLpressure(main_reservoir->getPressure());
    horn[CAB2]->step(t, dt);

    // Система подачи песка
    sand_system->setFLpressure(main_reservoir->getPressure());
    sand_system->step(t, dt);
    for (size_t i = 0; i < num_axis; ++i)
    {
        // Пересчёт трения колесо-рельс
        psi[i] = sand_system->getWheelRailFrictionCoeff(psi[i]);
    }
    // Пересчёт массы локомотива
    payload_coeff = sand_system->getSandLevel();
    setPayloadCoeff(payload_coeff);

    // Управление прожекторами
    for (auto i : {CAB1, CAB2})
    {
        spotlight[i]->setState(spotlight_low_tumbler[i].getState(),
                               spotlight_high_tumbler[i].getState());
        spotlight[i]->step(t, dt);
    }

    if (reg == nullptr)
        return;

    QString msg = "";
    msg += QString("v%1 kmh|").arg(velocity * 3.6, 10, 'f', 5);
    msg += QString("omega%1|").arg(wheel_omega[0], 10, 'f', 5);
    msg += QString("motor%1|").arg(motor[0]->getTorque(), 12, 'f', 5);
    reg->print(msg, t, dt);
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
    bool overload = false;

    for (auto ov_relay : overload_relay)
    {
        overload |= ov_relay->getState();
    }

    bool state_off = overload ||
                     controller[CAB1]->getState().pos_state[POS_BV] ||
                     controller[CAB2]->getState().pos_state[POS_BV];

    return !state_off;
}
