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

    QString tmp_qstr = window_title.c_str();
    if (cfg.getString(section, "WindowTitle", tmp_qstr))
    {
        window_title = tmp_qstr.toStdString();
    }

    cfg.getInt(section, "WindowX", window_x);
    cfg.getInt(section, "WindowY", window_y);
    cfg.getInt(section, "WindowWidth", window_width);
    cfg.getInt(section, "WindowHeight", window_height);

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

    cfg.getDouble(section, "zNear", zNear);
    cfg.getDouble(section, "ViewDistance", view_distance);
    cfg.getDouble(section, "FovY", fovy);
    cfg.getDouble(section, "FovYMin", fovy_min);
    cfg.getDouble(section, "FovYMax", fovy_max);
    cfg.getDouble(section, "PitchMin", pitch_min);
    cfg.getDouble(section, "PitchMax", pitch_max);

    cfg.getDouble(section, "MinCameraRotateSpeed", min_camera_rotate_speed);
    cfg.getDouble(section, "MaxCameraRotateSpeed", max_camera_rotate_speed);
    cfg.getDouble(section, "CameraRotateSpeed", camera_rotate_speed);

    cfg.getDouble(section, "MinCameraZoomPower", min_camera_zoom_power);
    cfg.getDouble(section, "MaxCameraZoomPower", max_camera_zoom_power);
    cfg.getDouble(section, "CameraZoomPower", camera_zoom_power);

    cfg.getDouble(section, "MinCameraMoveSpeed", min_camera_move_speed);
    cfg.getDouble(section, "MaxCameraMoveSpeed", max_camera_move_speed);
    cfg.getDouble(section, "CameraMoveSpeed", camera_move_speed);

    cfg.getFloat(section, "GizmoArrowLength", gizmo_arrow_length);
    cfg.getFloat(section, "GizmoArrowThickness", gizmo_arrow_thickness);
    cfg.getFloat(section, "GizmoXAxisColorR", gizmo_arrow_x_color.r);
    cfg.getFloat(section, "GizmoXAxisColorG", gizmo_arrow_x_color.g);
    cfg.getFloat(section, "GizmoXAxisColorB", gizmo_arrow_x_color.b);
    cfg.getFloat(section, "GizmoYAxisColorR", gizmo_arrow_y_color.r);
    cfg.getFloat(section, "GizmoYAxisColorG", gizmo_arrow_y_color.g);
    cfg.getFloat(section, "GizmoYAxisColorB", gizmo_arrow_y_color.b);
    cfg.getFloat(section, "GizmoZAxisColorR", gizmo_arrow_z_color.r);
    cfg.getFloat(section, "GizmoZAxisColorG", gizmo_arrow_z_color.g);
    cfg.getFloat(section, "GizmoZAxisColorB", gizmo_arrow_z_color.b);
    cfg.getFloat(section, "GizmoOpacity", gizmo_opacity);
    cfg.getBool(section, "GizmoToCenter", gizmo_to_center);

    cfg.getBool(section, "ShowWireframe", show_wireframe);

    cfg.getDouble(section, "GuiFontSize", gui_font_size);
    cfg.getBool(section, "IsGuiEditable", is_gui_editable);
    cfg.getBool(section, "ShowObjectsRef", show_objects_ref);
    cfg.getBool(section, "ShowRouteMap", show_route_map);
    cfg.getBool(section, "ShowControls", show_controls);
    cfg.getBool(section, "ShowCameraSettings", show_camera_settings);
    cfg.getBool(section, "ShowTopology", show_topology);

    cfg.getString(section, "KeyMoveCameraForward", tmp_qstr);
    key_move_camera_forward = static_cast<vsg::KeySymbol>(
        tmp_qstr.front().toLatin1());

    cfg.getString(section, "KeyMoveCameraBackward", tmp_qstr);
    key_move_camera_backward = static_cast<vsg::KeySymbol>(
        tmp_qstr.front().toLatin1());

    cfg.getString(section, "KeyMoveCameraLeft", tmp_qstr);
    key_move_camera_left = static_cast<vsg::KeySymbol>(
        tmp_qstr.front().toLatin1());

    cfg.getString(section, "KeyMoveCameraRight", tmp_qstr);
    key_move_camera_right = static_cast<vsg::KeySymbol>(
        tmp_qstr.front().toLatin1());

    cfg.getString(section, "KeyMoveObjects", tmp_qstr);
    key_move_objects = static_cast<vsg::KeySymbol>(
        tmp_qstr.front().toLatin1());
}
