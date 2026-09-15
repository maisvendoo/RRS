#include    <ep1m.h>

#include    <core/load_module.h>

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::initAutopilot(const QString& modules_dir,
                         const QString& custom_cfg_dir)
{
    // Модули автоведения
    for (auto cab_idx : {CAB1, CAB2})
    {
        Autopilot* autopilot = LOAD_MODULE(Autopilot,
            modules_dir + QDir::separator() +
            custom_modules_dir + QDir::separator() +
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

            auto_feedback[cab_idx] = new ep1m_feedback_t();
            autopilot->setFeedback(auto_feedback[cab_idx]);

            connect(autopilot, &Autopilot::sigInitTrainParams, this, &EP1m::slotInitTrainForAutopilot);

            this->autopilot.push_back(autopilot);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::slotInitTrainForAutopilot()
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
    if (km[CAB1]->isReversHandle())
    {
        prepareCabineForAutopilot(CAB1, CAB2);
    }

    if (km[CAB2]->isReversHandle())
    {
        prepareCabineForAutopilot(CAB2, CAB1);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::prepareCabineForAutopilot(int my_cab_idx, int other_cab_idx)
{
    // В нашей кабине

    // включаем ЭПТ
    if (!epb_control->stateReleaseLamp())
    {
        tumblers_panel[my_cab_idx]->getTumblerPtr(EP1MTumblersPanel::TUMBLER_EPT)->set();
    }

    // Включаем белые буферные
    tumblers[TUMBLER_BUFFERLIGHT_L][my_cab_idx].set();
    tumblers[TUMBLER_BUFFERLIGHT_R][my_cab_idx].set();
    // прожектор ярко
    tumblers[TUMBLER_SPOTLIGHT_HIGH][my_cab_idx].set();

    // Включаем подсветку приборов
    tumblers[TUMBLER_DEVICES_LIGHT][my_cab_idx].set();

    // В другой кабине
    tumblers[TUMBLER_BUFFERLIGHT_L][other_cab_idx].set();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::OnAutopilot()
{
    Vehicle::OnAutopilot();

    // Делаем автозапуск
    if (km[CAB1]->isReversHandle())
    {
        initAutostartProgram(CAB1);
        autopilot_switcher[CAB1].set();
    }

    if (km[CAB2]->isReversHandle())
    {
        initAutostartProgram(CAB2);
        autopilot_switcher[CAB2].set();
    }

    autoStartTimer->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EP1m::OffAutopilot()
{
    for (auto cab_idx : {CAB1, CAB2})
    {
        autopilot_switcher[cab_idx].reset();
    }
}
