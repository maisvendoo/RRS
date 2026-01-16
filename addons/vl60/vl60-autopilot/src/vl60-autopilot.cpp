#include    <vl60-autopilot.h>

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
    Autopilot::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::preStep(state_vector_t &Y, double t)
{
    auto_feedback = dynamic_cast<vl60_feedback_t *>(feedback);

    // Режем сигнал интегратора
    Y[0] = cut(Y[0], -1.0, 1.0);

    // Ошибка по скорости
    dv = v_ref - auto_feedback->v_cur;

    // Вычисляем задание по току ТЭД
    double I_ref = Imax * (Kp * dv + getY(0));

    // Обрезаем задание по максимальной уставке
    I_ref = cut(I_ref, 0.0, Imax);

    // Если ток упал ниже уставки
    if (auto_feedback->I_motor < I_ref - delta_I)
    {
        // + позиция
        plusPos();
    }

    // Если ток сильно выше уставки
    if (auto_feedback->I_motor > I_ref + delta_I)
    {
        // - позиция
        minusPos();
    }

    // Если превышаем скорость - мотаем вниз (возможно нужно  с переходом на торможение)
    if (dv < 0)
    {
        minusPos();
    }
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::onPressRB_Timeout()
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

    if (auto_feedback->cur_pos == 0)
    {
        auto_control->km_pos_ref = POS_FV;

        if (!delay->isStarted())
            delay->start();

        return;
    }

    // Если текущая позиция совпадает с предудущей - ручку в РВ
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
void VL60Autopilot::slotDelayTimer()
{
    delay->stop();
}

GET_AUTOPILOT(VL60Autopilot)
