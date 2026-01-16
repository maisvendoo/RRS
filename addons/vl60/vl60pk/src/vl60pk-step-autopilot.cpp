#include    <vl60pk.h>

#include    <alsn-ukbm.h>
#include    <ekg-8g.h>
#include    <kme-60-044.h>
#include    <dc-motor.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepAutopilot(double t, double dt)
{
    for (auto cab_idx : {CAB1, CAB2})
    {
        if (autopilot[cab_idx] == nullptr)
        {
            return;
        }

        // Включение и выключение автоведения
        autopilot_switcher[cab_idx].getState() ? autopilot[cab_idx]->on() : autopilot[cab_idx]->off();

        // Сигнал контроля бдительности от цепей ламп ПСС
        auto_feedback[cab_idx]->is_vigilance_control = static_cast<bool>(safety_device[cab_idx]->getStatePSS());
        // Текущая позиция ЭКГ
        auto_feedback[cab_idx]->cur_pos = main_controller->getPosition();
        auto_feedback[cab_idx]->km_pos = controller[cab_idx]->getMainPos();
        auto_feedback[cab_idx]->I_motor = motor[0]->getIa();
        auto_feedback[cab_idx]->v_cur = qAbs(velocity * Physics::kmh);

        // Принимаем сигналы обратной связи от оборудования
        autopilot[cab_idx]->setFeedback(auto_feedback[cab_idx]);

        // Выполняем шаг управления
        autopilot[cab_idx]->step(t, dt);

        // Получаем управляющие воздействия
        auto_control[cab_idx] = dynamic_cast<vl60_control_t *>(autopilot[cab_idx]->getControl());

        // Действия по управлению, только если автоведение активно
        if (autopilot[cab_idx]->isActive())
        {
            auto_control[cab_idx]->press_RB ? rb[cab_idx][RBS].set() : rb[cab_idx][RBS].reset();

            controller[cab_idx]->setMainHandlePos(auto_control[cab_idx]->km_pos_ref);
        }
    }
}
