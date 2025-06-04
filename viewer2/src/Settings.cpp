#include "RouteViewer.h"

#include "CfgReader.h"
#include "CLI11.hpp"
#include "cmd-line.h"

#include <QString>

#include <sstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadNetworkSettings(CfgReader& cfg, const QString& section)
{
    cfg.getString(section, "HostAddr", settings.tcp_config.host_addr);

    int port = 0;
    if (cfg.getInt(section, "port", port))
    {
        settings.tcp_config.port = static_cast<quint16>(port);
    }

    cfg.getInt(section, "ReconnectInteval", settings.tcp_config.reconnect_interval);
    cfg.getInt(section, "VehiclesPosUpdateInterval", settings.vehicles_pos_update_interval);
    cfg.getInt(section, "VehiclesStateUpdateInterval", settings.vehicles_state_update_interval);
    cfg.getInt(section, "VehicleControlledUpdateInterval", settings.vehicle_controled_update_interval);
    cfg.getInt(section, "ClientDelay", settings.client_delay);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadLoggerSettings(CfgReader& cfg, const QString& section)
{
    QString notifyLevel = "INFO";
    if (cfg.getString(section, "NotifyLevel", notifyLevel))
    {
        settings.notify_level = notifyLevel.toStdString();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadWindowSettings(CfgReader& cfg, const QString& section)
{
    QString name = "viewer";
    if (cfg.getString(section, "Name", name))
    {
        settings.name = name.toStdString();
    }

    cfg.getInt(section, "posX", settings.x);
    cfg.getInt(section, "posY", settings.y);
    cfg.getInt(section, "Width", settings.width);
    cfg.getInt(section, "Height", settings.height);

    int screenNumber = 0;
    cfg.getInt(section, "ScreenNumber", screenNumber);
    if (screenNumber >= 0)
    {
        settings.screen_number = screenNumber;
    }

    cfg.getBool(section, "FullScreen", settings.fullscreen);
    cfg.getBool(section, "VSync", settings.vsync);
    cfg.getBool(section, "WindowDecoration", settings.window_decoration);

    cfg.getBool(section, "DoubleBuffer", settings.double_buffer);
    cfg.getInt(section, "Samples", settings.samples);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadLightSettings(CfgReader& cfg, const QString& section)
{
    // Настройки теней
    cfg.getBool(section, "Shadow", settings.shadow);
    cfg.getDouble(section, "ShadowDistance", settings.shadow_distance);
    cfg.getInt(section, "ShadowCascade", settings.shadow_cascade);
    cfg.getInt(section, "ShadowResolution", settings.shadow_resolution);
    // Корректность значений
    if ((settings.shadow_distance < 0.5) ||
        (settings.shadow_cascade < 1) ||
        (settings.shadow_resolution < 1))
    {
        settings.shadow = false;
    }

    // Настройки освещения
    cfg.getDouble(section, "AmbientIntensity", settings.ambient_intensity);

    QString ambientColor = "1.0 1.0 1.0";
    if (cfg.getString(section, "AmbientColor", ambientColor))
    {
        std::istringstream stream(ambientColor.toStdString());
        stream >> settings.ambient_color.x
            >> settings.ambient_color.y
            >> settings.ambient_color.z;
    }

    cfg.getDouble(section, "SunIntensity", settings.sun_intensity);

    QString sunColor = "1.0 1.0 1.0";
    if (cfg.getString(section, "SunColor", sunColor))
    {
        std::istringstream stream(sunColor.toStdString());
        stream >> settings.sun_color.x
            >> settings.sun_color.y
            >> settings.sun_color.z;
    }

    QString sunDirection = "1.0 1.0 -1.0";
    if (cfg.getString(section, "SunDirection", sunDirection))
    {
        std::istringstream stream(sunDirection.toStdString());
        stream >> settings.sun_direction.x
            >> settings.sun_direction.y
            >> settings.sun_direction.z;
    }

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadCameraSettings(CfgReader& cfg, const QString& section)
{
    cfg.getDouble(section, "ViewDistance", settings.view_distance);
    cfg.getDouble(section, "zNear", settings.zNear);
//    cfg.getDouble(section, "zFar", settings.zFar);
    cfg.getDouble(section, "FovY", settings.fovy);
    cfg.getDouble(section, "FovYMin", settings.fovy_min);
    cfg.getDouble(section, "FovYMax", settings.fovy_max);
    cfg.getDouble(section, "PitchMin", settings.pitch_min);
    cfg.getDouble(section, "PitchMax", settings.pitch_max);
}

//------------------------------------------------------------------------------
// Настройки свободной камеры
//------------------------------------------------------------------------------
void RouteViewer::loadFreeCameraSettings(CfgReader& cfg, const QString& section)
{
    // Положение свободной камеры при запуске
    QString freeCamStart = "0.0 0.0 0.0";
    if (cfg.getString(section, "FreeCamStart", freeCamStart))
    {
        std::istringstream stream(freeCamStart.toStdString());
        stream >> settings.free_cam_start.x
            >> settings.free_cam_start.y
            >> settings.free_cam_start.z;
    }

    QString freeCamInitPos = "0.0 0.0 0.0";
    if (cfg.getString(section, "FreeCamInitPos", freeCamInitPos))
    {
        std::istringstream stream(freeCamInitPos.toStdString());
        stream >> settings.free_cam_init_pos.x
            >> settings.free_cam_init_pos.y
            >> settings.free_cam_init_pos.z;
    }

    cfg.getDouble(section, "FreeCamSpeedKeyboard", settings.free_cam_speed_keyboard);
    cfg.getDouble(section, "FreeCamSpeedMouse", settings.free_cam_speed_mouse);

    double freeCamSpeedCoeff = 1.0;
    cfg.getDouble(section, "FreeCamSpeedCoeff", freeCamSpeedCoeff);
    if (freeCamSpeedCoeff > 1.01)
    {
        settings.free_cam_speed_coeff = freeCamSpeedCoeff;
    }

    cfg.getDouble(section, "FreeCamRotKeyboard", settings.free_cam_rotate_keyboard);
    cfg.getDouble(section, "FreeCamRotMouse", settings.free_cam_rotate_keyboard);
    cfg.getDouble(section, "FreeCamHeightStep", settings.free_cam_height_step);

    double freeCamFovYCoeff = 1.0;
    cfg.getDouble(section, "FreeCamFovYCoeff", freeCamFovYCoeff);
    if (freeCamSpeedCoeff > 1.01)
    {
        settings.free_cam_fovy_coeff = freeCamFovYCoeff;
    }
}

//------------------------------------------------------------------------------
// Настройки камеры в кабине
//------------------------------------------------------------------------------
void RouteViewer::loadCabineCameraSettings(CfgReader& cfg, const QString& section)
{
    QString cabineCamInitPos = "0.0 0.0 0.0";
    if (cfg.getString(section, "CabineCamInitPos", cabineCamInitPos))
    {
        std::istringstream stream(cabineCamInitPos.toStdString());
        stream >> settings.cabine_default_pos.x
            >> settings.cabine_default_pos.y
            >> settings.cabine_default_pos.z;
    }

    cfg.getDouble(section, "CabineCamSpeedKeyboard", settings.cabine_speed_keyboard);
    cfg.getDouble(section, "CabineCamSpeedMouse", settings.cabine_speed_mouse);

    double cabineCamSpeedCoeff = 1.0;
    cfg.getDouble(section, "CabineCamSpeedCoeff", cabineCamSpeedCoeff);
    if (cabineCamSpeedCoeff > 1.01)
    {
        settings.cabine_speed_coeff = cabineCamSpeedCoeff;
    }

    cfg.getDouble(section, "CabineCamRotKeyboard", settings.cabine_rotate_keyboard);
    cfg.getDouble(section, "CabineCamRotMouse", settings.cabine_rotate_mouse);
    cfg.getDouble(section, "CabineCamHeightStep", settings.cabine_height_step);

    double cabineCamFovYCoeff = 1.0;
    cfg.getDouble(section, "CabineCamFovYCoeff", cabineCamFovYCoeff);
    if (cabineCamFovYCoeff > 1.01)
    {
        settings.cabine_fovy_coeff = cabineCamFovYCoeff;
    }

    cfg.getDouble(section, "CabineCamVerticalShiftMin", settings.cabine_z_min);
    cfg.getDouble(section, "CabineCamVerticalShiftMax", settings.cabine_z_max);
}

//------------------------------------------------------------------------------
// Настройки внешней камеры
//------------------------------------------------------------------------------
void RouteViewer::loadExternalCameraSettings(CfgReader& cfg, const QString& section)
{
    QString extCamInitPos = "0.0 0.0 0.0";
    if (cfg.getString(section, "ExtCamInitPos", extCamInitPos))
    {
        std::istringstream stream(extCamInitPos.toStdString());
        stream >> settings.ext_cam_init_pos.x
            >> settings.ext_cam_init_pos.y
            >> settings.ext_cam_init_pos.z;
    }

    cfg.getDouble(section, "ExtCamInitAngleH", settings.ext_cam_init_angle_H);
    cfg.getDouble(section, "ExtCamInitAngleV", settings.ext_cam_init_angle_V);
    cfg.getDouble(section, "ExtCamInitDist", settings.ext_cam_init_distance);
    cfg.getDouble(section, "ExtCamSpeedKeyboard", settings.ext_cam_speed_keyboard);
    cfg.getDouble(section, "ExtCamSpeedMouse", settings.ext_cam_speed_mouse);

    double extCamSpeedCoeff = 1.0;
    cfg.getDouble(section, "ExtCamSpeedCoeff", extCamSpeedCoeff);
    if (extCamSpeedCoeff > 1.01)
    {
        settings.ext_cam_speed_coeff = extCamSpeedCoeff;
    }

    cfg.getDouble(section, "ExtCamRotKeyboard", settings.ext_cam_rotate_keyboard);
    cfg.getDouble(section, "ExtCamRotMouse", settings.ext_cam_rotate_mouse);
    cfg.getDouble(section, "ExtCamHeightStep", settings.ext_cam_height_step);

    double extCamDistCoeff = 1.0;
    cfg.getDouble(section, "ExtCamDistCoeff", extCamDistCoeff);
    if (extCamDistCoeff > 1.01)
    {
        settings.ext_cam_dist_coeff = extCamDistCoeff;
    }

    cfg.getDouble(section, "ExtCamDistMin", settings.ext_cam_dist_min);
}

//------------------------------------------------------------------------------
// Настройки следящей камеры
//------------------------------------------------------------------------------
void RouteViewer::loadFollowCameraSettings(CfgReader& cfg, const QString& section)
{
    cfg.getDouble(section, "FollowCamShiftRight", settings.follow_cam_init_shift_right);
    cfg.getDouble(section, "FollowCamShiftUp", settings.follow_cam_init_shift_up);
    cfg.getDouble(section, "FollowCamFwdVelocityCoeff", settings.follow_cam_fwd_velocity_coeff);
    cfg.getDouble(section, "FollowCamSpeedKeyboard", settings.follow_cam_speed_keyboard);
    cfg.getDouble(section, "FollowCamSpeedMouse", settings.follow_cam_speed_mouse);

    double followCamSpeedCoeff = 1.0;
    cfg.getDouble(section, "FollowCamSpeedCoeff", followCamSpeedCoeff);
    if (followCamSpeedCoeff > 1.01)
    {
        settings.follow_cam_speed_coeff = followCamSpeedCoeff;
    }

    double followCamFovYCoeff = 1.0;
    cfg.getDouble(section, "FollowCamFovYCoeff", followCamFovYCoeff);
    if (followCamFovYCoeff > 1.01)
    {
        settings.follow_cam_fovy_coeff = followCamFovYCoeff;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int RouteViewer::overrideSettingsByCommandLine(int argc, char* argv[])
{
    cmd_line_t cmd_line;

    CLI::App app("Viewer");
    argv = app.ensure_utf8(argv);

    app.add_option("--host-addr", cmd_line.host_addr);
    app.add_option("--port", cmd_line.port);
    app.add_option("--width", cmd_line.width);
    app.add_option("--height", cmd_line.height);
    app.add_option("--fullscreen", cmd_line.fullscreen);
    app.add_option("--notify-level", cmd_line.notify_level);

    CLI11_PARSE(app, argc, argv);

    if (cmd_line.host_addr)
    {
        settings.tcp_config.host_addr = cmd_line.host_addr->c_str();
    }

    if (cmd_line.port)
    {
        settings.tcp_config.port = static_cast<quint16>(cmd_line.port.value());
    }

    if (cmd_line.width)
    {
        settings.width = cmd_line.width.value();
    }

    if (cmd_line.height)
    {
        settings.height = cmd_line.height.value();
    }

    if (cmd_line.fullscreen)
    {
        settings.fullscreen = cmd_line.fullscreen.value();
    }

    if (cmd_line.notify_level)
    {
        settings.notify_level = cmd_line.notify_level.value();
    }

    return 0;
}
