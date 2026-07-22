#include "Settings.h"

#include "Action.h"
#include "Journal.h"
#include "KeyBinding.h"

#include <CfgReader.h>

#include <vsg/ui/KeyEvent.h>

#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <Qt>
#include <QtTypes>

#include <map>
#include <string>

static constexpr const char* action_setting_names[TOTAL_ACTIONS] = {
    "MoveCameraForward",
    "MoveCameraBackward",
    "MoveCameraLeft",
    "MoveCameraRight",
    "MoveObjects",
    "RotateObjects",
    "ScaleObjects",
    "CopyObjects",
    "PasteObjects",
    "HideObjects",
    "ShowObjects",
    "DeleteObjects",
    "UndoCommand",
    "RedoCommand",
    "SaveRoute",
    "ChangeProjectionMatrix"
};

settings_t::settings_t() = default;

void settings_t::read(const std::string& cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path.c_str()))
    {
        return;
    }

    window_settings.read(cfg);

    const std::map<std::string, vsg::KeyModifier> modifier_name_map = {
        {"alt", vsg::MODKEY_Alt},
        {"ctrl", vsg::MODKEY_Control},
        {"shift", vsg::MODKEY_Shift}
    };

    for (int i = 0; i < TOTAL_ACTIONS; ++i)
    {
        const Action action = static_cast<Action>(i);
        const char* const setting_name = action_setting_names[i];

        QString line;

        if (!cfg.getString("Keys", setting_name, line))
        {
            Journal::instance()->error(QString("Failed to find key setting %1")
                .arg(setting_name));
            continue;
        }

        line = line.toLower();

        const QStringList strings = line.split(QRegularExpression("[ +]"),
            Qt::SkipEmptyParts);

        const qsizetype string_size = strings.size();
        if (string_size <= 0)
        {
            continue;
        }

        KeyBinding& key_binding = key_bindings[action];
        key_binding.key = static_cast<vsg::KeySymbol>(
            strings.back().front().toLatin1());

        for (qsizetype i = 0; i < string_size - 1; ++i)
        {
            const auto found_it = modifier_name_map.find(strings[i].toStdString());
            if (found_it != modifier_name_map.cend())
            {
                key_binding.modifiers |= found_it->second;
            }
        }
    }
}
