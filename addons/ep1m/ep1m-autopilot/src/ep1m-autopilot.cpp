#include    <ep1m-autopilot.h>

#include    <core/get_module.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP1mAutopilot::EP1mAutopilot() : Autopilot(nullptr)
{
    connect(km_delay, &Timer::process, this, &EP1mAutopilot::slotDelayKM);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EP1mAutopilot::~EP1mAutopilot()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
auto_control_t *EP1mAutopilot::getControl()
{
    return auto_control;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1mAutopilot::step(double t, double dt)
{
    km_delay->step(t, dt);
    brake_control->step(t, dt);

    Autopilot::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1mAutopilot::initAutoBrakeControl(const QString &config_name,
                                         const QString &custom_cfg_dir)
{
    brake_control->read_config(config_name, custom_cfg_dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1mAutopilot::press_RB()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1mAutopilot::release_RB()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1mAutopilot::load_config(CfgReader &cfg)
{
    Autopilot::load_config(cfg);

    QString secName = "Device";

    cfg.getDouble(secName, "Imax", Imax);
    cfg.getDouble(secName, "Kp", Kp);
    cfg.getDouble(secName, "Ks", Ks);
    cfg.getBool(secName, "DisableEDB", edb_disable);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1mAutopilot::preStep(state_vector_t &Y, double t)
{
    // Приводим общую структуру обратной связи к нашему типу
    auto_feedback = dynamic_cast<ep1m_feedback_t *>(feedback);

    if (auto_feedback == nullptr)
    {
        return;
    }

    // Вычисляем ошибку по скорости
    dv = v_ref - feedback->v_cur;

    // Вычисляем абсолютную скорость проскальзывания
    double dv_s = pf(feedback->v_tau - feedback->v_cur);

    // Задание по  току
    double I_ref = Imax * (Kp * dv - Ks * dv_s);

    if (edb_disable)
        I_ref = cut(I_ref, 0.0, Imax);
    else
        I_ref = cut(I_ref, -Imax, Imax);

    // Выбираем режим работы привода
    mode_pos_old = mode_pos;
    mode_pos = tree_pos_relay(I_ref, -5.0, 5.0);

    traction_control(mode_pos, I_ref / I_ref_max);

    // Задаем скорость для регулятора
    auto_control->v_level = v_ref / v_constr;

    // Управляем пневматикой
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

    auto_control->press_RB = auto_feedback->is_vigilance_control;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int8_t EP1mAutopilot::tree_pos_relay(double x, double x_min, double x_max)
{
    if (x > x_max)
    {
        return 1;
    }

    if (x < x_min)
    {
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1mAutopilot::traction_control(int8_t mode_pos, double level)
{
    if (km_delay->isStarted())
    {
        return;
    }

    if (!is_motion_allowed)
    {
        auto_control->mode_pos = 0;

        if (!km_delay->isStarted())
        {
            km_delay->start();
        }

        return;
    }

    // Перекладываем ручку на другой режим - только через ноль
    if (mode_pos * mode_pos_old < 0 && !auto_feedback->km_is_zero)
    {
        auto_control->mode_pos = 0;

        if (!km_delay->isStarted())
        {
            km_delay->start();
        }

        return;
    }

    if (mode_pos == 1 && !auto_feedback->is_traction_ON)
    {
        if (auto_feedback->km_is_zero)
        {
            auto_control->mode_pos = 1;
        }
        else
        {
            auto_control->mode_pos = 0;
        }

        if (!km_delay->isStarted())
        {
            km_delay->start();
        }

        return;
    }

    if (mode_pos == -1 && !auto_feedback->is_brake_ON)
    {
        if (auto_feedback->km_is_zero)
        {
            auto_control->mode_pos = -1;
        }
        else
        {
            auto_control->mode_pos = 0;
        }

        if (!km_delay->isStarted())
        {
            km_delay->start();
        }

        return;
    }

    // Задаем уровень тяги/ЭДТ
    if (level > 0)
    {
        if (!lock_traction)
            auto_control->level = level;
        else
            auto_control->level = 0;
    }
    else
    {
        auto_control->level = level;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1mAutopilot::slotDelayKM()
{
    km_delay->stop();
}

GET_MODULE(EP1mAutopilot)
