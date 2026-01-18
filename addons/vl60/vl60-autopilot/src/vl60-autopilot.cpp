#include    <vl60-autopilot.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60Autopilot::VL60Autopilot() : Autopilot(nullptr)
{
    connect(delay, &Timer::process, this, &VL60Autopilot::slotDelayTimer);
    connect(krm_handle_timer, &Timer::process, this, &VL60Autopilot::slotBrakeCraneHandle);
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
/*void VL60Autopilot::setFeedback(auto_feedback_t *feedback)
{
    auto_feedback = dynamic_cast<vl60_feedback_t *>(feedback);
}*/

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::step(double t, double dt)
{
    delay->step(t, dt);
    krm_handle_timer->step(t, dt);
    Autopilot::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::preStep(state_vector_t &Y, double t)
{
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
    double I_ref = Imax * (Kp * dv + getY(0));

    // Обрезаем задание по максимальной уставке
    I_ref = cut(I_ref, 0.0, Imax);

    // Блокирование тяги по давлению в ТЦ
    if (auto_feedback->pBC > 0.04)
    {
        lock_traction = true;
    }
    else
    {
        // Если тяга заблокирована но скорость не упала сильно
        if (lock_traction && dv < 10.0)
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

    // Если превышаем скорость - мотаем вниз до упора
    if (dv < -0.25)
    {
        auto_control->km_pos_ref = POS_ZERO;        
    }

    if (auto_feedback->is_EPB_on)
    {
        stepEPB(dv, t);
    }
    else
    {
        stepPB(dv, t);
    }

    stepKVT();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
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
void VL60Autopilot::brakeStep(double p_charge, double dp)
{
    lock_traction = true;

    double delta_pEQ = cut((brake_step + 1) * dp, 0.0, 0.15);

    if ( auto_feedback->pEQ > p_charge -  delta_pEQ)
    {
        if (!krm_handle_timer->isStarted())
        {
            auto_control->krm_pos = 5;
        }
    }
    else
    {
        if (auto_control->krm_pos == 5)
        {
            auto_control->krm_pos = 3;
            brake_step++;
            krm_handle_timer->start();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::brakeRelease(double p_charge)
{
    if (is_disable_release)
    {
        return;
    }

    brake_step = 0;
    krm_handle_timer->stop();

    if (auto_feedback->pEQ < p_charge - 0.02)
    {
        auto_control->krm_pos = 0;
    }
    else
    {
        auto_control->krm_pos = 1;
        lock_traction = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::stepPB(double dv, double t)
{
    if (dv < -0.5)
    {
        brakeStep(auto_feedback->p_charge, 0.06);
    }

    if (dv >= 5.0)
    {
        brakeRelease(auto_feedback->p_charge);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::stepEPB(double dv, double t)
{
    // Максимальное превышение над программной скоростью
    const double dVminus = -0.5;
    // Максимальное снижение скорости относительно программной
    const double dVplus = 3.0;

    // Превышаем программую скорость
    if (dv < dVminus)
    {
        // Ступень торможения
        setBrakeCranePos(4);
        // Запрет тяги
        lock_traction = true;
    }

    // Скорость в дупустимом коридоре
    if ( dv >= -dVminus && dv <= dVplus)
    {
        // Ставим в перекрышу, при условии что в ТЦ минимут 1 кгс
        if (lock_traction && auto_feedback->pBC >= 0.1)
        {
            setBrakeCranePos(3);
        }
    }

    // Скорость упала ниже коридора, нет запрета отпуска, запрещена тяга
    if (dv > dVplus && !is_disable_release && lock_traction)
    {
        // Отпускаем

        // Последняя ступень отпуска I положением
        if ( auto_feedback->pBC < 0.1)
        {
            if (auto_feedback->pEQ < auto_feedback->p_charge + 0.02)
            {
                // Первая ступень только нет завышения в УР
                setBrakeCranePos(0);
            }
            else // иначе - II положение
            {
                setBrakeCranePos(1);
            }
        }
        else // и если не последняя ступень отпуска - отпускаем II положением
            setBrakeCranePos(1);
    }

    if (auto_feedback->pEQ >= auto_feedback->p_charge + 0.02 && auto_control->krm_pos == 0)
    {
        setBrakeCranePos(1);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::stepKVT()
{
    // Если движение запрещено - зажимаем КВТ на полную
    if (!is_motion_allowed)
    {
        // ставим КВТ на полное торможение
        auto_control->kvt_pos = 1.0;

        // Снимаем запрет отпуска
        is_disable_release = false;

        // отпускаем состав если кран в перекрыше
        if (auto_control->krm_pos == 3 || auto_control->krm_pos == 2)
        {
            setBrakeCranePos(0);
        }
    }
    else // Иначе - отпускаем
    {
        auto_control->kvt_pos = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::setBrakeCranePos(int pos)
{
    if (!krm_handle_timer->isStarted())
    {
        auto_control->krm_pos = pos;
        krm_handle_timer->start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::slotDelayTimer()
{
    delay->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::slotBrakeCraneHandle()
{
    krm_handle_timer->stop();
}

GET_AUTOPILOT(VL60Autopilot)
