#include "editor/KeyBindings.h"

#include "editor/Action.h"

#include <CfgReader.h>
#include <Journal.h>

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
    "SaveRoute"
};

static const std::map<std::string, vsg::KeyModifier> modkey_map = {
    {"alt", vsg::MODKEY_Alt},
    {"ctrl", vsg::MODKEY_Control},
    {"shift", vsg::MODKEY_Shift}
};

void KeyBindings::read(CfgReader& cfg)
{
    const QString section = "Keys";

    for (int i = 0; i < TOTAL_ACTIONS; ++i)
    {
        const char* const setting_name = action_setting_names[i];

        QString line;
        if (!cfg.getString(section, setting_name, line))
        {
            Journal::instance()->error(QString("Failed to find key binding %1")
                .arg(setting_name));
            continue;
        }

        line = line.toLower();

        const QStringList strings = line.split(QRegularExpression("[ +]"),
            Qt::SkipEmptyParts);

        const qsizetype strings_size = strings.size();
        if (strings_size <= 0)
        {
            continue;
        }

        KeyBinding& binding = bindings[i];
        binding.key = static_cast<vsg::KeySymbol>(
            strings.back().front().toLatin1());

        for (qsizetype j = 0; j < strings_size - 1; ++j)
        {
            const auto found_it = modkey_map.find(strings[j].toStdString());
            if (found_it != modkey_map.cend())
            {
                binding.modifiers |= found_it->second;
            }
        }
    }
}
