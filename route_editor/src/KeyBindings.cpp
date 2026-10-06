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

static std::array<std::string, TOTAL_ACTIONS> get_action_setting_names()
{
    std::array<std::string, TOTAL_ACTIONS> action_setting_names;

    action_setting_names[ACTION_MOVE_CAMERA_FORWARD]      = "MoveCameraForward";
    action_setting_names[ACTION_MOVE_CAMERA_BACKWARD]     = "MoveCameraBackward";
    action_setting_names[ACTION_MOVE_CAMERA_LEFT]         = "MoveCameraLeft";
    action_setting_names[ACTION_MOVE_CAMERA_RIGHT]        = "MoveCameraRight";
    action_setting_names[ACTION_TRANSLATE_OBJECTS]        = "MoveObjects";
    action_setting_names[ACTION_ROTATE_OBJECTS]           = "RotateObjects";
    action_setting_names[ACTION_SCALE_OBJECTS]            = "ScaleObjects";
    action_setting_names[ACTION_COPY_OBJECTS]             = "CopyObjects";
    action_setting_names[ACTION_PASTE_OBJECTS]            = "PasteObjects";
    action_setting_names[ACTION_HIDE_OBJECTS]             = "HideObjectsCommand";
    action_setting_names[ACTION_SHOW_OBJECTS]             = "ShowObjects";
    action_setting_names[ACTION_DELETE_OBJECTS]           = "DeleteObjects";
    action_setting_names[ACTION_UNDO_COMMAND]             = "UndoCommand";
    action_setting_names[ACTION_REDO_COMMAND]             = "RedoCommand";
    action_setting_names[ACTION_SAVE_ROUTE]               = "SaveRoute";
    action_setting_names[ACTION_SWAP_PROJECTION_MATRIX] = "ChangeProjectionMatrix";

    return action_setting_names;
};

static const std::map<std::string, vsg::KeyModifier> modifier_map = {
    {"alt", vsg::MODKEY_Alt},
    {"ctrl", vsg::MODKEY_Control},
    {"shift", vsg::MODKEY_Shift}
};

KeyBindings::KeyBindings()
{
    for (int i = 0; i < TOTAL_ACTIONS; ++i)
    {
        keys[i] = static_cast<vsg::KeySymbol>(0);
        modifiers[i] = 0;
    }
}

void KeyBindings::read(CfgReader& cfg)
{
    const auto action_setting_names = get_action_setting_names();

    for (int i = 0; i < TOTAL_ACTIONS; ++i)
    {
        const char* action_setting_name = action_setting_names[i].c_str();

        QString line;
        if (!cfg.getString("Keys", action_setting_name, line))
        {
            Journal::instance()->error(QString("Failed to find key binding %1")
                .arg(action_setting_name));
            continue;
        }

        line = line.toLower();

        const QStringList strings = line.split(QRegularExpression("[ +]"),
            Qt::SkipEmptyParts);

        if (strings.size() <= 0)
        {
            continue;
        }

        keys[i] = static_cast<vsg::KeySymbol>(strings.back().front().toLatin1());

        for (const auto& qstr : strings)
        {
            const auto found_it = modifier_map.find(qstr.toStdString());
            if (found_it != modifier_map.end())
            {
                modifiers[i] = found_it->second;
            }
        }
    }
}
