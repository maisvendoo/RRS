#include    <vl60pk.h>
#include    <QDir>
#include    <epb-2line-control.h>
#include    <trigger-control.h>
#include    <automatic-train-stop.h>

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

            connect(autopilot, &Autopilot::sigInitTrainLength, this, &VL60pk::slotInitTrainLengh);
        }

        this->autopilot.push_back(autopilot);

        auto_feedback[cab_idx] = new vl60_feedback_t();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::slotInitTrainLengh()
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
        if (epk[CAB1]->isKeyOn())
        {
            epb_switch[CAB1].set();
        }

        if (epk[CAB2]->isKeyOn())
        {
            epb_switch[CAB2].set();
        }
    }
}
