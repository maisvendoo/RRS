#include "Settings.h"
#include "Action.h"
#include "KeyBinding.h"

#include <CfgReader.h>

#include <cstdint>
#include <iterator>
#include <qnamespace.h>
#include <qregularexpression.h>
#include <vsg/ui/KeyEvent.h>

#include <QString>

#include <string>

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

    section = "Camera";

    cfg.getDouble(section, "zNear", zNear);
    cfg.getDouble(section, "ViewDistance", view_distance);
    cfg.getDouble(section, "FovY", fovy);
    cfg.getDouble(section, "FovYMin", fovy_min);
    cfg.getDouble(section, "FovYMax", fovy_max);
    cfg.getDouble(section, "InitialHeight", camera_initial_height);
    cfg.getDouble(section, "MoveSpeed", camera_move_speed);
    cfg.getDouble(section, "MinMoveSpeed", min_camera_move_speed);
    cfg.getDouble(section, "MaxMoveSpeed", max_camera_move_speed);
    cfg.getDouble(section, "RotateSpeed", camera_rotate_speed);
    cfg.getDouble(section, "MinRotateSpeed", min_camera_rotate_speed);
    cfg.getDouble(section, "MaxRotateSpeed", max_camera_rotate_speed);
    cfg.getDouble(section, "ZoomPower", camera_zoom_power);
    cfg.getDouble(section, "MinZoomPower", min_camera_zoom_power);
    cfg.getDouble(section, "MaxZoomPower", max_camera_zoom_power);

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

    cfg.getDouble(section, "FontSize", gui_font_size);
    cfg.getBool(section, "IsEditable", is_gui_editable);
    cfg.getBool(section, "ShowObjectsRef", show_objects_ref);
    cfg.getBool(section, "ShowRouteMap", show_route_map);
    cfg.getBool(section, "ShowControls", show_controls);
    cfg.getBool(section, "ShowCameraSettings", show_camera_settings);
    cfg.getBool(section, "ShowTopology", show_topology);

    section = "Keys";

    using ActionSettingNameMap = std::map<Action, const char*>;
    using ActionSettingNamePair = ActionSettingNameMap::value_type;

    const ActionSettingNamePair action_setting_name_map_data[] = {
        {ACTION_MOVE_CAMERA_FORWARD,  "MoveCameraForward"},
        {ACTION_MOVE_CAMERA_BACKWARD, "MoveCameraBackward"},
        {ACTION_MOVE_CAMERA_LEFT,     "MoveCameraLeft"},
        {ACTION_MOVE_CAMERA_RIGHT,    "MoveCameraRight"},

        {ACTION_MOVE_OBJECTS,   "MoveObjects"},
        {ACTION_ROTATE_OBJECTS, "RotateObjects"},
        {ACTION_SCALE_OBJECTS,  "ScaleObjects"},
        {ACTION_COPY_OBJECTS,   "CopyObjects"},
        {ACTION_PASTE_OBJECTS,  "PasteObjects"},

        {ACTION_UNDO_COMMAND, "UndoCommand"},
        {ACTION_REDO_COMMAND, "RedoCommand"},
    };

    static_assert(sizeof action_setting_name_map_data /
        sizeof(ActionSettingNamePair) == TOTAL_ACTIONS);

    const ActionSettingNameMap action_setting_name_map(
        std::begin(action_setting_name_map_data),
        std::end(action_setting_name_map_data));

    const std::map<std::string, EditorKeyModifier>
    key_modifier_setting_name_map = {
        {"lshift",   EDITOR_KEY_MODIFIER_SHIFT_L},
        {"rshift",   EDITOR_KEY_MODIFIER_SHIFT_R},
        {"shift",    EDITOR_KEY_MODIFIER_SHIFT_ANY},
        {"anyshift", EDITOR_KEY_MODIFIER_SHIFT_ANY},

        {"lctrl",   EDITOR_KEY_MODIFIER_CTRL_L},
        {"rctrl",   EDITOR_KEY_MODIFIER_CTRL_R},
        {"ctrl",    EDITOR_KEY_MODIFIER_CTRL_ANY},
        {"anyctrl", EDITOR_KEY_MODIFIER_CTRL_ANY},

        {"lalt",   EDITOR_KEY_MODIFIER_ALT_L},
        {"ralt",   EDITOR_KEY_MODIFIER_ALT_R},
        {"alt",    EDITOR_KEY_MODIFIER_ALT_ANY},
        {"anyalt", EDITOR_KEY_MODIFIER_ALT_ANY}
    };

    for (const auto& [action, setting_name] : action_setting_name_map)
    {
        QString line;
        cfg.getString(section, setting_name, line);

        line = line.toLower();

        const auto strings = line.split(QRegularExpression("[ +]"),
            Qt::SkipEmptyParts);

        const auto string_size = strings.size();
        if (string_size <= 0)
        {
            continue;
        }

        std::uint32_t modifiers = 0;

        for (auto i = decltype(string_size){0}; i < string_size - 1; ++i)
        {
            const auto str = strings[i].toStdString();
            const auto found = key_modifier_setting_name_map.find(str);
            if (found != key_modifier_setting_name_map.cend())
            {
                modifiers |= found->second;
            }
        }

        key_bindings[action].key = static_cast<vsg::KeySymbol>(strings.back().front().toLatin1());
        key_bindings[action].modifiers = modifiers;
    }
}
