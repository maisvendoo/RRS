#include    <vl60pk.h>
#include    <QDir>
#include    <epb-2line-control.h>
#include    <trigger-control.h>
#include    <automatic-train-stop.h>
#include    <kme-60-044.h>
#include    <brake-crane.h>
#include    <loco-crane.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initAutopilot(const QString& modules_dir,
                           const QString& custom_cfg_dir)
{
    // Модули автоведения
    for (auto cab_idx : {CAB1, CAB2})
    {
        Autopilot *autopilot = loadAutopilot(modules_dir + QDir::separator()
                                         + "vl60" + QDir::separator() +
                                         "vl60-autopilot");

        if (autopilot != nullptr)
        {
            autopilot->read_config("vl60-autopilot", custom_cfg_dir);
            autopilot_switcher[cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
            autopilot_switcher[cab_idx].setKeySymbolOn(KEY_F);
            autopilot_switcher[cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
            autopilot_switcher[cab_idx].setKeySymbolOff(KEY_F);
            autopilot_switcher[cab_idx].setControl(&pressed_keys);

            connect(autopilot, &Autopilot::sigInitTrainLength, this, &VL60pk::slotInitTrainForAutopilot);
        }

        this->autopilot.push_back(autopilot);

        auto_feedback[cab_idx] = new vl60_feedback_t();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::slotInitTrainForAutopilot()
{
    double train_len = 0;

    emit sigGetTrainLength(train_idx, train_len);

    for (auto cab_idx : {CAB1, CAB2})
    {
        autopilot[cab_idx]->setTrainLength(train_len);
    }

    // Кое-какие другие действия при активации автоведения

    // Если выключен ЭПТ, то включаем его
    if (!epb_control->stateReleaseLamp())
    {
        if (controller[CAB1]->isReversHandle())
        {
            prepareCabineForAutopilot(CAB1, CAB2);
        }

        if (controller[CAB2]->isReversHandle())
        {
            prepareCabineForAutopilot(CAB2, CAB1);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::prepareCabineForAutopilot(int my_cab_idx, int other_cab_idx)
{
    // В нашей кабине

    // включаем ЭПТ
    epb_switch[my_cab_idx].set();
    // зажигаем белые буферные
    P_bufferlight_L_tumbler[my_cab_idx].set();
    P_bufferlight_R_tumbler[my_cab_idx].set();
    // прожектор на ярко
    spotlight_high_tumbler[my_cab_idx].set();
    // подсветка приборов
    P_light_devices_tumbler[my_cab_idx].set();

    // инициализируем структуру управляющего воздействия текущим положением
    // органов управления
    auto_control[my_cab_idx]->press_RB = rb[my_cab_idx][RBS].getState();
    auto_control[my_cab_idx]->spotlight_ON = spotlight_low_tumbler[my_cab_idx].getState();
    auto_control[my_cab_idx]->km_pos_ref = controller[my_cab_idx]->getMainPos();
    auto_control[my_cab_idx]->krm_pos = brake_crane[my_cab_idx]->getHandlePosition();
    auto_control[my_cab_idx]->kvt_pos = loco_crane[my_cab_idx]->getHandlePosition();

    // В другой кабине

    // левый буферный белый
    P_bufferlight_L_tumbler[other_cab_idx].set();
}
