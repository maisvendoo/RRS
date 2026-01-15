#include    <vl60pk.h>
#include    <QDir>

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
        }

        this->autopilot.push_back(autopilot);

        auto_feedback[cab_idx] = new vl60_feedback_t();
    }
}
