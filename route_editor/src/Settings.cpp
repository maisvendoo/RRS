#include "Settings.h"
#include "KeyBindings.h"

#include <CfgReader.h>

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

    const auto get_key_binding_setting = [&](const char* const name,
        KeyBinding& key_binding) -> void
    {
        QString line;
        cfg.getString(section, name, line);

        line = line.toLower();

        const auto strings = line.split(QRegularExpression("[ +]"),
            Qt::SkipEmptyParts);

        const auto strings_size = strings.size();

        int modifiers = 0;

        for (auto i = decltype(strings_size){0}; i < strings_size - 1; ++i)
        {
            static const std::map<std::string, MyKeyModifier> modifiers_map = {
                {"lshift", MY_KEY_MODIFIER_SHIFT_L},
                {"rshift", MY_KEY_MODIFIER_SHIFT_R},
                {"shift", MY_KEY_MODIFIER_SHIFT_ANY},
                {"anyshift", MY_KEY_MODIFIER_SHIFT_ANY},
                {"lctrl", MY_KEY_MODIFIER_CTRL_L},
                {"rctrl", MY_KEY_MODIFIER_CTRL_R},
                {"ctrl", MY_KEY_MODIFIER_CTRL_ANY},
                {"anyctrl", MY_KEY_MODIFIER_CTRL_ANY},
                {"lalt", MY_KEY_MODIFIER_ALT_L},
                {"ralt", MY_KEY_MODIFIER_ALT_R},
                {"alt", MY_KEY_MODIFIER_ALT_ANY},
                {"anyalt", MY_KEY_MODIFIER_ALT_ANY}
            };

            const QString str = strings[i];
            for (const auto& [label, modifier] : modifiers_map)
            {
                if (str.toStdString() == label)
                {
                    modifiers |= modifier;
                }
            }
        }

        key_binding.modifiers = modifiers;
        key_binding.key = static_cast<vsg::KeySymbol>(
            strings.back().front().toLatin1());
    };

    get_key_binding_setting("MoveCameraForward", key_move_camera_forward);
    get_key_binding_setting("MoveCameraBackward", key_move_camera_backward);
    get_key_binding_setting("MoveCameraLeft", key_move_camera_left);
    get_key_binding_setting("MoveCameraRight", key_move_camera_right);
    get_key_binding_setting("MoveObjects", key_move_objects);
    get_key_binding_setting("RotateObjects", key_rotate_objects);
}
