#include    <ep1m.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::stepAutopilot(double t, double dt)
{
    if (autopilot.empty())
    {
        return;
    }

    double v_lim = 0;
    double v_lim_next = 0;
    double limit_dist = 0;
    double signal_dist = 0;
    ALSN alsn_code = ALSN::NO_CODE;

    int cab_idx = 0;

    if (km[CAB1]->isReversHandle())
    {
        v_lim = speedmap_fwd->getCurrentLimit();
        v_lim_next = speedmap_fwd->getNextLimit();
        limit_dist = speedmap_fwd->getNextLimitDistance();
        alsn_code = coil_ALSN_fwd->getCode();
        signal_dist = coil_ALSN_fwd->getNextSignalDistance();

        cab_idx = CAB1;
    }

    if (km[CAB2]->isReversHandle())
    {
        v_lim = speedmap_bwd->getCurrentLimit();
        v_lim_next = speedmap_bwd->getNextLimit();
        limit_dist = speedmap_bwd->getNextLimitDistance();
        alsn_code = coil_ALSN_bwd->getCode();
        signal_dist = coil_ALSN_bwd->getNextSignalDistance();

        cab_idx = CAB2;
    }

    if (autopilot[cab_idx] == nullptr)
    {
        return;
    }

    // Включение и выключение автоведения
    autopilot_switcher[cab_idx].getState() ? autopilot[cab_idx]->on() : autopilot[cab_idx]->off();

    // TODO: обратная связь конкретно от данного локомотива

    // Сигнал контроля бдительности от цепей КЛУБ
    auto_feedback[cab_idx]->is_vigilance_control = klub_BEL->isCheckVigilanse();
    auto_feedback[cab_idx]->I_motor = trac_motor[TRAC_MOTOR1]->getAncorCurrent();
    auto_feedback[cab_idx]->v_cur = qAbs(velocity * Physics::kmh);
    auto_feedback[cab_idx]->v_tau = qAbs(wheel_omega[0] * wheel_diameter[0] / 2.0 * Physics::kmh);
    auto_feedback[cab_idx]->v_lim = klub_BEL->getCurrentSpeedLimit();
    auto_feedback[cab_idx]->v_lim_next = klub_BEL->getNextSpeedLimit();
    auto_feedback[cab_idx]->limit_dist = limit_dist;
    auto_feedback[cab_idx]->alsn_code = alsn_code;
    auto_feedback[cab_idx]->signal_dist = signal_dist;
    auto_feedback[cab_idx]->pBC = brake_mech[TROLLEY_FWD]->getBCpressure();
    auto_feedback[cab_idx]->pEQ = brake_crane[cab_idx]->getERpressure();
    auto_feedback[cab_idx]->p_charge = charge_press;
    auto_feedback[cab_idx]->is_EPB_on = epb_control->stateReleaseLamp();
    auto_feedback[cab_idx]->km_is_zero = km[cab_idx]->isZero();

    // Проверяем состояние сбора схемы
    auto_feedback[cab_idx]->is_LC_ON = msud_input.is_traction || msud_input.is_brake; //true;
    auto_feedback[cab_idx]->is_traction_ON = msud_input.is_traction;
    auto_feedback[cab_idx]->is_brake_ON = msud_input.is_brake;

    /*for (auto lc : fast_switch)
    {
        auto_feedback[cab_idx]->is_LC_ON = auto_feedback[cab_idx]->is_LC_ON && lc->getContactState(0);
    }*/

    // Принимаем сигналы обратной связи от оборудования
    autopilot[cab_idx]->setFeedback(auto_feedback[cab_idx]);

    // Выполняем шаг управления
    autopilot[cab_idx]->step(t, dt);

    // Получаем управляющие воздействия
    auto_control[cab_idx] = dynamic_cast<ep1m_control_t *>(autopilot[cab_idx]->getControl());

    // Действия по управлению, только если автоведение активно
    if (autopilot[cab_idx]->isActive())
    {
        // TODO: действия по управления конкретно этим локомотивом

        // Проверка бдительности
        auto_control[cab_idx]->press_RB ? tumblers[BUTTON_RBS][cab_idx].set() : tumblers[BUTTON_RBS][cab_idx].reset();

        km[cab_idx]->setMode(auto_control[cab_idx]->mode_pos);
        km[cab_idx]->setLevel(auto_control[cab_idx]->level);
        km[cab_idx]->setVrefLevel(auto_control[cab_idx]->v_level);

        // Управление КрМ
        brake_crane[cab_idx]->setHandlePosition(auto_control[cab_idx]->krm_pos);

        // Управление КВТ
        loco_crane[cab_idx]->setHandlePosition(auto_control[cab_idx]->kvt_pos);

        auto_control[cab_idx]->spotlight_ON ? tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].set() : tumblers[TUMBLER_SPOTLIGHT_LOW][cab_idx].reset();

        horn[cab_idx]->lockManualControl(true);
        horn[cab_idx]->setSvistokOn(auto_control[cab_idx]->whistle);
        horn[cab_idx]->setTifonOn(auto_control[cab_idx]->typhoid);
    }
    else
    {
        horn[cab_idx]->lockManualControl(false);
    }
}
