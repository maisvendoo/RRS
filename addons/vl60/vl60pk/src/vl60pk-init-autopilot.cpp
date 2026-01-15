#include    <vl60pk.h>
#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initAutopilot(const QString& modules_dir,
                           const QString& custom_cfg_dir)
{
    // Модуль автоведения
    autopilot = loadAutopilot(modules_dir + QDir::separator()
                              + "vl60" + QDir::separator() +
                              "vl60-autopilot");

    if (autopilot != nullptr)
    {
        autopilot->read_config("vl60-autopilot", custom_cfg_dir);
        autopilot_switcher.setKeyModifierOn(MODIFIER_OnlyAlt);
        autopilot_switcher.setKeySymbolOn(KEY_F);
        autopilot_switcher.setKeyModifierOff(MODIFIER_OnlyAlt);
        autopilot_switcher.setKeySymbolOff(KEY_F);
        autopilot_switcher.setControl(&pressed_keys);
    }
}
