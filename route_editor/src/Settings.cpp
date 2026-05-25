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

#include <iterator>
#include <map>
#include <string>

settings_t::settings_t()
    : gui_font_size(20.0f)
    , is_gui_editable(false)
    , show_objects_ref(true)
    , show_route_map(false)
    , show_stations_conf(true)
    , show_waypoints_conf(false)
    , show_key_bindings(true)
    , show_camera_settings(false)
    , show_topology(false)
    , show_selected_objects_properties(true)
    , show_commands(true)
{
}

void settings_t::read(const std::string& cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path.c_str()))
    {
        return;
    }

    window_settings.read(cfg);
    scene_settings.read(cfg);
    camera_settings.read(cfg);
    gizmo_settings.read(cfg);

    QString section = "GUI";

    cfg.getFloat(section, "FontSize", gui_font_size);
    cfg.getBool(section, "IsEditable", is_gui_editable);
    cfg.getBool(section, "ShowObjectsRef", show_objects_ref);
    cfg.getBool(section, "ShowRouteMap", show_route_map);
    cfg.getBool(section, "ShowStationsConf", show_stations_conf);
    cfg.getBool(section, "ShowWaypointsConf", show_waypoints_conf);
    cfg.getBool(section, "ShowKeyBindings", show_key_bindings);
    cfg.getBool(section, "ShowCameraSettings", show_camera_settings);
    cfg.getBool(section, "ShowTopology", show_topology);
    cfg.getBool(section, "ShowSelectedObjectsProperties", show_selected_objects_properties);
    cfg.getBool(section, "ShowCommands", show_commands);

    section = "Keys";

    using ActionSettingNameMap = std::map<Action, const char*>;
    using ActionSettingNamePair = ActionSettingNameMap::value_type;

    constexpr ActionSettingNamePair action_setting_name_map_data[] = {
        {ACTION_MOVE_CAMERA_FORWARD, "MoveCameraForward"},
        {ACTION_MOVE_CAMERA_BACKWARD, "MoveCameraBackward"},
        {ACTION_MOVE_CAMERA_LEFT, "MoveCameraLeft"},
        {ACTION_MOVE_CAMERA_RIGHT, "MoveCameraRight"},
        {ACTION_TRANSLATE_OBJECTS, "MoveObjects"},
        {ACTION_ROTATE_OBJECTS, "RotateObjects"},
        {ACTION_SCALE_OBJECTS, "ScaleObjects"},
        {ACTION_COPY_OBJECTS, "CopyObjects"},
        {ACTION_PASTE_OBJECTS, "PasteObjects"},
        {ACTION_HIDE_OBJECTS, "HideObjects"},
        {ACTION_SHOW_OBJECTS, "ShowObjects"},
        {ACTION_DELETE_OBJECTS, "DeleteObjects"},
        {ACTION_UNDO_COMMAND, "UndoCommand"},
        {ACTION_REDO_COMMAND, "RedoCommand"},
        {ACTION_SAVE_ROUTE, "SaveRoute"}
    };

    static_assert(sizeof action_setting_name_map_data /
        sizeof(ActionSettingNamePair) == TOTAL_ACTIONS);

    const ActionSettingNameMap action_setting_name_map(
        std::begin(action_setting_name_map_data),
        std::end(action_setting_name_map_data));

    const std::map<std::string, vsg::KeyModifier>
    key_modifier_setting_name_map = {
        {"shift", vsg::MODKEY_Shift},
        {"ctrl", vsg::MODKEY_Control},
        {"alt", vsg::MODKEY_Alt}
    };

    for (const auto& [action, setting_name] : action_setting_name_map)
    {
        QString line;

        if (!cfg.getString(section, setting_name, line))
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
            const auto found_it = key_modifier_setting_name_map.find(
                strings[i].toStdString());

            if (found_it != key_modifier_setting_name_map.cend())
            {
                key_binding.modifiers |= found_it->second;
            }
        }
    }
}
