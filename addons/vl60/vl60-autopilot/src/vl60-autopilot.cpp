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
void VL60Autopilot::setFeedback(auto_feedback_t *feedback)
{
    auto_feedback = dynamic_cast<vl60_feedback_t *>(feedback);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::step(double t, double dt)
{
    double I_ref = 300.0;

    I_ref = cut(I_ref, 0.0, Imax);

    if (auto_feedback->I_motor < I_ref - delta_I)
    {
        plusPos();
    }

    if (auto_feedback->I_motor > I_ref + delta_I)
    {
        minusPos();
    }

    delay->step(t, dt);
    Autopilot::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Imax", Imax);
    cfg.getDouble(secName, "DeltaI", delta_I);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60Autopilot::vigilance_control(double t, double dt)
{
    // Есть сигнал контроля бдительности
    if (auto_feedback->is_vigilance_control)
    {
        if (!rb_timer->isStarted())
        {
            // Жмем РБ
            auto_control->press_RB = true;
            // Запускаем таймер
            rb_timer->start();
        }
    }
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
