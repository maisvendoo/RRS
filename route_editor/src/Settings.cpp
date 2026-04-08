#include "Settings.h"

#include "Action.h"
#include "KeyBinding.h"

#include <CfgReader.h>

#include <vsg/ui/KeyEvent.h>

#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <Qt>
#include <QtTypes>

#include <cstdio>
#include <iterator>
#include <map>
#include <string>

settings_t::settings_t()
    : window_title("Route Editor")
    , window_x(50)
    , window_y(50)
    , window_width(1280)
    , window_height(720)
    , screen_number(0)
    , fullscreen(false)
    , vsync(true)
    , double_buffer(true)
    , samples(1)
    , num_lights(200)
    , zNear(0.1)
    , view_distance(2000.0)
    , fovy(60.0)
    , fovy_min(2.0)
    , fovy_max(100.0)
    , camera_initial_height(0.0)
    , camera_move_speed(100.0)
    , camera_rotate_speed(3.0)
    , camera_zoom_power(250.0)
    , gizmo_arrow_length(5.0f)
    , gizmo_arrow_thickness(0.1f)
    , gizmo_arrow_x_color(1.0f, 0.0f, 0.0f)
    , gizmo_arrow_y_color(0.0f, 1.0f, 0.0f)
    , gizmo_arrow_z_color(0.0f, 0.0f, 1.0f)
    , gizmo_opacity(1.0f)
    , gizmo_to_center(false)
    , gui_font_size(20.0f)
    , is_gui_editable(false)
    , show_objects_ref(true)
    , show_route_map(false)
    , show_controls(true)
    , show_camera_settings(false)
    , show_topology(false)
{
}

void settings_t::read(const std::string& cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path.c_str()))
    {
        return;
    }

    QString section = "Window";

    QString tmp_qstr = window_title.c_str();
    if (cfg.getString(section, "Title", tmp_qstr))
    {
        window_title = tmp_qstr.toStdString();
    }

    cfg.getInt(section, "xPos", window_x);
    cfg.getInt(section, "yPos", window_y);
    cfg.getInt(section, "Width", window_width);
    cfg.getInt(section, "Height", window_height);

    int tmp_int = screen_number;
    cfg.getInt(section, "ScreenNumber", tmp_int);
    if (tmp_int >= 0)
    {
        screen_number = tmp_int;
    }

    cfg.getBool(section, "FullScreen", fullscreen);
    cfg.getBool(section, "VSync", vsync);
    cfg.getBool(section, "DoubleBuffer", double_buffer);
    cfg.getInt(section, "Samples", samples);
    cfg.getInt(section, "NumLights", num_lights);

    section = "Camera";

    cfg.getDouble(section, "zNear", zNear);
    cfg.getDouble(section, "ViewDistance", view_distance);
    cfg.getDouble(section, "FovY", fovy);
    cfg.getDouble(section, "FovYMin", fovy_min);
    cfg.getDouble(section, "FovYMax", fovy_max);
    cfg.getDouble(section, "InitialHeight", camera_initial_height);
    cfg.getDouble(section, "MoveSpeed", camera_move_speed);
    cfg.getDouble(section, "RotateSpeed", camera_rotate_speed);
    cfg.getDouble(section, "ZoomPower", camera_zoom_power);

    section = "Gizmo";

    cfg.getFloat(section, "ArrowLength", gizmo_arrow_length);
    cfg.getFloat(section, "ArrowThickness", gizmo_arrow_thickness);
    cfg.getFloat(section, "XAxisColorR", gizmo_arrow_x_color.r);
    cfg.getFloat(section, "XAxisColorG", gizmo_arrow_x_color.g);
    cfg.getFloat(section, "XAxisColorB", gizmo_arrow_x_color.b);
    cfg.getFloat(section, "YAxisColorR", gizmo_arrow_y_color.r);
    cfg.getFloat(section, "YAxisColorG", gizmo_arrow_y_color.g);
    cfg.getFloat(section, "YAxisColorB", gizmo_arrow_y_color.b);
    cfg.getFloat(section, "ZAxisColorR", gizmo_arrow_z_color.r);
    cfg.getFloat(section, "ZAxisColorG", gizmo_arrow_z_color.g);
    cfg.getFloat(section, "ZAxisColorB", gizmo_arrow_z_color.b);
    cfg.getFloat(section, "Opacity", gizmo_opacity);
    cfg.getBool(section, "ToCenter", gizmo_to_center);

    section = "GUI";

    cfg.getFloat(section, "FontSize", gui_font_size);
    cfg.getBool(section, "IsEditable", is_gui_editable);
    cfg.getBool(section, "ShowObjectsRef", show_objects_ref);
    cfg.getBool(section, "ShowRouteMap", show_route_map);
    cfg.getBool(section, "ShowControls", show_controls);
    cfg.getBool(section, "ShowCameraSettings", show_camera_settings);
    cfg.getBool(section, "ShowTopology", show_topology);

    section = "Keys";

    using ActionSettingNameMap = std::map<Action, const char*>;
    using ActionSettingNamePair = ActionSettingNameMap::value_type;

    constexpr ActionSettingNamePair action_setting_name_map_data[] = {
        {ACTION_MOVE_CAMERA_FORWARD, "MoveCameraForward"},
        {ACTION_MOVE_CAMERA_BACKWARD, "MoveCameraBackward"},
        {ACTION_MOVE_CAMERA_LEFT, "MoveCameraLeft"},
        {ACTION_MOVE_CAMERA_RIGHT, "MoveCameraRight"},
        {ACTION_MOVE_OBJECTS, "MoveObjects"},
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

    const std::map<std::string, EditorKeyModifier>
    key_modifier_setting_name_map = {
        {"shift", EDITOR_KEY_MODIFIER_SHIFT},
        {"ctrl", EDITOR_KEY_MODIFIER_CTRL},
        {"alt", EDITOR_KEY_MODIFIER_ALT}
    };

    for (const auto& [action, setting_name] : action_setting_name_map)
    {
        QString line;

        if (!cfg.getString(section, setting_name, line))
        {
            // TODO: Replace on Journal
            std::fprintf(stderr, "Failed to find key setting %s\n",
                setting_name);

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
