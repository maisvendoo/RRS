#include "editor/Action.h"

#include <CfgReader.h>
#include <Journal.h>

#include <vsg/ui/KeyEvent.h>

#include <QRegularExpression>
#include <QString>

Actions::Actions()
{
    descriptions[ACTION_MOVE_CAMERA_FORWARD] = "Camera: move forward";
    setting_names[ACTION_MOVE_CAMERA_FORWARD] = "MoveCameraForward";

    descriptions[ACTION_MOVE_CAMERA_BACKWARD] = "Camera: move backward";
    setting_names[ACTION_MOVE_CAMERA_BACKWARD] = "MoveCameraBackward";

    descriptions[ACTION_MOVE_CAMERA_LEFT] = "Camera: move left";
    setting_names[ACTION_MOVE_CAMERA_LEFT] = "MoveCameraLeft";

    descriptions[ACTION_MOVE_CAMERA_RIGHT] = "Camera: move right";
    setting_names[ACTION_MOVE_CAMERA_RIGHT] = "MoveCameraRight";

    descriptions[ACTION_TRANSLATE_OBJECTS] = "Objects: Translate";
    setting_names[ACTION_TRANSLATE_OBJECTS] = "MoveObjects";

    descriptions[ACTION_ROTATE_OBJECTS] = "Objects: Rotate";
    setting_names[ACTION_ROTATE_OBJECTS] = "RotateObjects";

    descriptions[ACTION_SCALE_OBJECTS] = "Objects: Scale";
    setting_names[ACTION_SCALE_OBJECTS] = "ScaleObjects";

    descriptions[ACTION_COPY_OBJECTS] = "Objects: Copy";
    setting_names[ACTION_COPY_OBJECTS] = "CopyObjects";

    descriptions[ACTION_PASTE_OBJECTS] = "Objects: Paste";
    setting_names[ACTION_PASTE_OBJECTS] = "PasteObjects";

    descriptions[ACTION_HIDE_OBJECTS] = "Objects: Hide";
    setting_names[ACTION_HIDE_OBJECTS] = "HideObjects";

    descriptions[ACTION_SHOW_OBJECTS] = "Objects: Show";
    setting_names[ACTION_SHOW_OBJECTS] = "ShowObjects";

    descriptions[ACTION_DELETE_OBJECTS] = "Objects: Delete";
    setting_names[ACTION_DELETE_OBJECTS] = "DeleteObjects";

    descriptions[ACTION_UNDO_COMMAND] = "Undo command";
    setting_names[ACTION_UNDO_COMMAND] = "UndoCommand";

    descriptions[ACTION_REDO_COMMAND] = "Redo command";
    setting_names[ACTION_REDO_COMMAND] = "RedoCommand";

    descriptions[ACTION_SAVE_ROUTE] = "Save route";
    setting_names[ACTION_SAVE_ROUTE] = "SaveRoute";

    descriptions[ACTION_SWAP_PROJECTION_MATRIX] = "Camera: change projection matrix";
    setting_names[ACTION_SWAP_PROJECTION_MATRIX] = "ChangeProjectionMatrix";

    for (int i = 0; i < TOTAL_ACTIONS; ++i)
    {
        keys[i] = static_cast<vsg::KeySymbol>(0);
        modifiers[i] = 0;
    }
}

static const std::map<std::string, vsg::KeyModifier> modifier_map = {
    {"alt", vsg::MODKEY_Alt},
    {"ctrl", vsg::MODKEY_Control},
    {"shift", vsg::MODKEY_Shift}
};

void Actions::read(CfgReader& cfg)
{
    for (int i = 0; i < TOTAL_ACTIONS; ++i)
    {
        std::string& setting_name = setting_names[i];

        QString line;
        if (!cfg.getString("Keys", setting_name.c_str(), line))
        {
            Journal::instance()->error(QString("Failed to find key binding %1")
                .arg(setting_name));
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
