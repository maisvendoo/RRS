#include "Settings.h"

#include <CfgReader.h>

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

    const QString section = "Editor";

    QString temp_string = window_title.c_str();
    if (cfg.getString(section, "WindowTitle", temp_string))
    {
        window_title = temp_string.toStdString();
    }

    cfg.getInt(section, "WindowX", window_x);
    cfg.getInt(section, "WindowY", window_y);
    cfg.getInt(section, "WindowWidth", window_width);
    cfg.getInt(section, "WindowHeight", window_height);

    int temp_int = screen_number;
    cfg.getInt(section, "ScreenNumber", temp_int);
    if (temp_int >= 0)
    {
        screen_number = temp_int;
    }

    cfg.getBool(section, "FullScreen", fullscreen);
    cfg.getBool(section, "VSync", vsync);

    cfg.getBool(section, "DoubleBuffer", double_buffer);
    cfg.getInt(section, "Samples", samples);

    cfg.getDouble(section, "zNear", zNear);
    cfg.getDouble(section, "ViewDistance", view_distance);
    cfg.getDouble(section, "FovY", fovy);
    cfg.getDouble(section, "FovYMin", fovy_min);
    cfg.getDouble(section, "FovYMax", fovy_max);
    cfg.getDouble(section, "PitchMin", pitch_min);
    cfg.getDouble(section, "PitchMax", pitch_max);

    cfg.getFloat(section, "GizmoArrowLength", gizmo_arrow_length);
    cfg.getFloat(section, "GizmoArrowThickness", gizmo_arrow_thickness);

    const char* field_names[] = {
        "GizmoXAxisColorR",
        "GizmoXAxisColorG",
        "GizmoXAxisColorB",
        "GizmoYAxisColorR",
        "GizmoYAxisColorG",
        "GizmoYAxisColorB",
        "GizmoZAxisColorR",
        "GizmoZAxisColorG",
        "GizmoZAxisColorB"
    };

    float* color_components[] = {
        &gizmo_arrow_x_color.r,
        &gizmo_arrow_x_color.g,
        &gizmo_arrow_x_color.b,
        &gizmo_arrow_y_color.r,
        &gizmo_arrow_y_color.g,
        &gizmo_arrow_y_color.b,
        &gizmo_arrow_z_color.r,
        &gizmo_arrow_z_color.g,
        &gizmo_arrow_z_color.b
    };

    for (int i = 0; i < 9; ++i)
    {
        cfg.getFloat(section, field_names[i], *color_components[i]);
    }

    cfg.getFloat(section, "GizmoOpacity", gizmo_opacity);

    cfg.getDouble(section, "GuiFontSize", gui_font_size);
    cfg.getBool(section, "IsGuiEditable", is_gui_editable);
    cfg.getBool(section, "ShowObjectsRef", show_objects_ref);
    cfg.getBool(section, "ShowRouteMap", show_route_map);
    cfg.getBool(section, "ShowControls", show_controls);
    cfg.getBool(section, "ShowCameraSettings", show_camera_settings);
    cfg.getBool(section, "ShowTopology", show_topology);

    cfg.getDouble(section, "MinCameraRotateSpeed", min_camera_rotate_speed);
    cfg.getDouble(section, "MaxCameraRotateSpeed", max_camera_rotate_speed);
    cfg.getDouble(section, "CameraRotateSpeed", camera_rotate_speed);

    cfg.getDouble(section, "MinCameraZoomPower", min_camera_zoom_power);
    cfg.getDouble(section, "MaxCameraZoomPower", max_camera_zoom_power);
    cfg.getDouble(section, "CameraZoomPower", camera_zoom_power);

    cfg.getDouble(section, "MinCameraMoveSpeed", min_camera_move_speed);
    cfg.getDouble(section, "MaxCameraMoveSpeed", max_camera_move_speed);
    cfg.getDouble(section, "CameraMoveSpeed", camera_move_speed);

    cfg.getString(section, "KeyMoveCameraForward", temp_string);
    key_move_camera_forward = static_cast<vsg::KeySymbol>(temp_string.front().toLatin1());

    cfg.getString(section, "KeyMoveCameraBackward", temp_string);
    key_move_camera_backward = static_cast<vsg::KeySymbol>(temp_string.front().toLatin1());

    cfg.getString(section, "KeyMoveCameraLeft", temp_string);
    key_move_camera_left = static_cast<vsg::KeySymbol>(temp_string.front().toLatin1());

    cfg.getString(section, "KeyMoveCameraRight", temp_string);
    key_move_camera_right = static_cast<vsg::KeySymbol>(temp_string.front().toLatin1());
}
