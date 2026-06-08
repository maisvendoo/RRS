#include    <vl60-autopilot.h>

#include    <core/get_module.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60Autopilot::VL60Autopilot() : Autopilot(nullptr)
{
    connect(delay, &Timer::process, this, &VL60Autopilot::slotDelayTimer);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60Autopilot::~VL60Autopilot()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
auto_control_t *VL60Autopilot::getControl()
{
    return auto_control;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::step(double t, double dt)
{
    delay->step(t, dt);
    brake_control->step(t, dt);
    Autopilot::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::initAutoBrakeControl(const QString &config_name,
                                         const QString &custom_cfg_dir)
{
    brake_control->read_config(config_name, custom_cfg_dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::preStep(state_vector_t &Y, double t)
{
    (void)t;

    // Приводим общую структуру обратной связи к нашему типу
    auto_feedback = dynamic_cast<vl60_feedback_t *>(feedback);

    if (auto_feedback == nullptr)
    {
        return;
    }

    // Режем сигнал интегратора
    Y[0] = cut(Y[0], -1.0, 1.0);

    // Ошибка по скорости
    dv = v_ref - auto_feedback->v_cur;

    // Вычисляем задание по току ТЭД
    double kp = Kp * train_mass / ref_mass;

    double dv_s = pf(feedback->v_tau - feedback->v_cur);

    double I_ref = Imax * (kp * dv - Ks * dv_s + getY(0));

    I_ref = cut(I_ref, 0.0, Imax);

    // Блокирование тяги по давлению в ТЦ
    if (auto_feedback->pBC > 0.04)
    {
        lock_traction = true;
    }
    else
    {
        // Если тяга заблокирована но скорость не упала сильно
        if (lock_traction && dv < 5.0)
            lock_traction = true; // продолжаем блокировать тягу
        else
            lock_traction = false;
    }

    // Если ток упал ниже уставки
    if (auto_feedback->I_motor < I_ref - delta_I)
    {
        if (!lock_traction)
        {
            // + позиция
            plusPos();
        }
    }

    // Если ток сильно выше уставки
    if (auto_feedback->I_motor > I_ref + delta_I)
    {
        // - позиция
        minusPos();
    }

    if (auto_feedback->I_motor >= 500.0)
    {
        sand_ON();
    }

    // Если превышаем скорость - мотаем вниз до упора
    if (dv < -dV_traction_off)
    {
        auto_control->km_pos_ref = POS_ZERO;
    }

    brake_control->setBrakePressures(auto_feedback->pEQ,
                                     auto_feedback->pBC,
                                     auto_feedback->p_charge);

    brake_control->setFeedback(auto_feedback->v_cur, dist_target, a_brake, accel_meter->value());

    brake_control->step_control(auto_feedback->is_EPB_on,
                                dv,
                                is_motion_allowed,
                                lock_traction,
                                is_disable_release);

    autopilot_brake_control_state_t bc_state = brake_control->getControlState();

    auto_control->krm_pos = bc_state.brake_crane_pos_ref;
    auto_control->kvt_pos = bc_state.loco_crane_pos_ref;

    // Управляем прожектором - включаем когда разрешено движение
    auto_control->spotlight_ON = is_motion_allowed;

    wheel_slim_process();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    (void)Y;
    (void)t;

    dYdt[0] = Ki * dv;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::load_config(CfgReader &cfg)
{
    Autopilot::load_config(cfg);

    QString secName = "Device";

    cfg.getDouble(secName, "Imax", Imax);
    cfg.getDouble(secName, "DeltaI", delta_I);
    cfg.getDouble(secName, "Kp", Kp);
    cfg.getDouble(secName, "Ki", Ki);
    cfg.getDouble(secName, "Ks", Ks);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::release_RB()
{
    auto_control->press_RB = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::press_RB()
{
    auto_control->press_RB = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::plusPos()
{
    if (!is_active)
        return;

    if (delay->isStarted())
    {
        return;
    }

    if (auto_feedback->cur_pos == 37)
    {
        auto_control->km_pos_ref = POS_FP;

        if (!delay->isStarted())
            delay->start();

        return;
    }

    // При разомкнутых ЛК
    if (!auto_feedback->is_LC_ON)
    {
        // Если рукоятка не в 0, то ставим её туда
        if (auto_feedback->km_pos != POS_ZERO)
            auto_control->km_pos_ref = POS_ZERO;
        else // иначе - переводим в АВ, для замыкания ЛК
            auto_control->km_pos_ref = POS_AV;

        // Задержка
        if (!delay->isStarted())
            delay->start();

        // Уходим - к набору пока не готовы
        return;
    }

    // Если текущая позиция совпадает с предудущей - ручку в РП
    if (auto_feedback->cur_pos == prev_pos)
    {
        if (auto_feedback->km_pos == POS_FP)
        {
            auto_control->km_pos_ref = POS_RP;

            if (!delay->isStarted())
                delay->start();
        }
        else
        {
            auto_control->km_pos_ref = POS_FP;

            if (!delay->isStarted())
                delay->start();
        }
    }
    else // иначе - ручку в ФП, обновляем предыдущую позицию
    {
        prev_pos = auto_feedback->cur_pos;
        auto_control->km_pos_ref = POS_FP;

        if (!delay->isStarted())
            delay->start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::minusPos()
{
    if (!is_active)
        return;

    if (delay->isStarted())
        return;

    // Если рукоятка уже в нуле - не фиг её вообще дергать - мотаем до нуля
    if (auto_feedback->km_pos == POS_ZERO)
    {
        return;
    }

    // Если смотаны все позиции, ставим рукоятку в ноль
    if (auto_feedback->cur_pos == 0)
    {
        auto_control->km_pos_ref = POS_ZERO;

        if (!delay->isStarted())
            delay->start();

        return;
    }

    // Если текущая позиция совпадает с предыдущей - ручку в РВ
    if (auto_feedback->cur_pos == prev_pos)
    {
        if (auto_feedback->km_pos == POS_FV)
        {
            auto_control->km_pos_ref = POS_RV;

            if (!delay->isStarted())
                delay->start();
        }
        else
        {
            auto_control->km_pos_ref = POS_FV;

            if (!delay->isStarted())
                delay->start();
        }
    }
    else // иначе - ручку в ФВ, обновляем предыдущую позицию
    {
        prev_pos = auto_feedback->cur_pos;
        auto_control->km_pos_ref = POS_FV;

        if (!delay->isStarted())
            delay->start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::sand_ON()
{
    if (!sand_timer->isStarted())
    {
        sand_timer->start();
        auto_control->sand_ON = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::sand_OFF()
{
    auto_control->sand_ON = false;
}

//------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void VL60Autopilot::wheel_slim_process()
{
    if (auto_feedback->v_tau - auto_feedback->v_cur >= 1.0)
    {
        sand_ON();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::slotDelayTimer()
{
    delay->stop();
}

GET_MODULE(VL60Autopilot)
