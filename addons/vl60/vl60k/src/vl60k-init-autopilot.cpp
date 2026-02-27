#include    <vl60k.h>

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
void VL60k::initAutopilot(const QString& modules_dir,
                           const QString& custom_cfg_dir)
{
    // Модули автоведения
    for (auto cab_idx : {CAB1, CAB2})
    {
        Autopilot *autopilot = loadAutopilot(modules_dir + QDir::separator()
                                             + custom_modules_dir + QDir::separator() +
                                             autopilot_module_name);

        if (autopilot != nullptr)
        {
            autopilot->setVehicleIndex(model_idx);
            autopilot->read_config(autopilot_config_name, custom_cfg_dir);
            autopilot->initAutoBrakeControl(autopilot_config_name, custom_cfg_dir);
            autopilot_switcher[cab_idx].setKeyModifierOn(MODIFIER_OnlyAlt);
            autopilot_switcher[cab_idx].setKeySymbolOn(KEY_F);
            autopilot_switcher[cab_idx].setKeyModifierOff(MODIFIER_OnlyAlt);
            autopilot_switcher[cab_idx].setKeySymbolOff(KEY_F);
            autopilot_switcher[cab_idx].setControl(&pressed_keys);

            connect(autopilot, &Autopilot::sigInitTrainParams, this, &VL60k::slotInitTrainForAutopilot);
        }

        this->autopilot.push_back(autopilot);

        auto_feedback[cab_idx] = new vl60_feedback_t();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::slotInitTrainForAutopilot()
{
    double train_len = 0;
    double train_mass = 0;

    emit sigGetTrainParams(train_idx, train_len, train_mass);

    for (auto cab_idx : {CAB1, CAB2})
    {
        autopilot[cab_idx]->setTrainLength(train_len);
        autopilot[cab_idx]->setTrainMass(train_mass);
    }

    // Кое-какие другие действия при активации автоведения
    if (controller[CAB1]->isReversHandle())
    {
        prepareCabineForAutopilot(CAB1, CAB2);
    }

    if (controller[CAB2]->isReversHandle())
    {
        prepareCabineForAutopilot(CAB2, CAB1);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::prepareCabineForAutopilot(int my_cab_idx, int other_cab_idx)
{
    // В нашей кабине

    // зажигаем белые буферные
    P_bufferlight_L_tumbler[my_cab_idx].set();
    P_bufferlight_R_tumbler[my_cab_idx].set();
    // прожектор на ярко
    spotlight_high_tumbler[my_cab_idx].set();
    // подсветка приборов
    P_light_devices_tumbler[my_cab_idx].set();

    // В другой кабине

    // левый буферный белый
    P_bufferlight_L_tumbler[other_cab_idx].set();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::OnAutopilot()
{
    Vehicle::OnAutopilot();

    // Делаем автозапуск
    if (controller[CAB1]->isReversHandle())
    {
        initAutostartProgram(CAB1);
        autopilot_switcher[CAB1].set();
    }

    if (controller[CAB2]->isReversHandle())
    {
        initAutostartProgram(CAB2);
        autopilot_switcher[CAB2].set();
    }

    autoStartTimer->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60k::OffAutopilot()
{
    for (auto cab_idx : {CAB1, CAB2})
    {
        autopilot_switcher[cab_idx].reset();
    }
}
