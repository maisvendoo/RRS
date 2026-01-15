#include    <vl60pk.h>

#include    <alsn-ukbm.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepAutopilot(double t, double dt)
{
    if (autopilot == nullptr)
    {
        return;
    }

    // Включение и выключение автоведения
    autopilot_switcher.getState() ? autopilot->on() : autopilot->off();

    // Сигнал контроля бдительности от цепей ламп ПСС
    auto_feedback->is_vigilance_control = static_cast<bool>(safety_device[CAB1]->getStatePSS())
        || static_cast<bool>(safety_device[CAB2]->getStatePSS());


    // Принимаем сигналы обратной связи от оборудования
    autopilot->setFeedback(auto_feedback);

    // Выполняем шаг управления
    autopilot->step(t, dt);

    // Получаем управляющие воздействия
    auto_control = dynamic_cast<vl60_control_t *>(autopilot->getControl());

    // Жмем физическую РБС от втопилота
    if (autopilot->isActive())
    {
        for (auto cab_idx : {CAB1, CAB2})
        {
            auto_control->press_RB ? rb[cab_idx][RBS].set() : rb[cab_idx][RBS].reset();
        }
    }
}
