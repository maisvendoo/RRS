#include "RouteViewer.h"

#include "CfgReader.h"

#include <iostream>
#include <vsg/utils/CommandLine.h>

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
    cfg.getBool(section, "ShowServerAddr", settings.tcp_config.show_server_addr);

    cfg.getDouble(section, "TrainProfileBackward", settings.train_profile_backward);
    cfg.getDouble(section, "TrainProfileForward", settings.train_profile_forward);
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
void RouteViewer::loadModelsSettings(CfgReader& cfg, const QString& section)
{
    int targetPagedLODs = 0;
    cfg.getInt(section, "TargetPagedLODs", targetPagedLODs);
    if (targetPagedLODs > 0)
    {
        settings.targetPagedLODs = targetPagedLODs;
    }

    cfg.getDouble(section, "CullingScreenRatio", settings.cullingScreenHeightRatio);
    settings.cullingScreenHeightRatio = std::clamp(settings.cullingScreenHeightRatio, 0.0, 1.0);

    int threads = -1;
    cfg.getInt(section, "ReadThreads", threads);
    if (threads >= 1)
    {
        settings.read_threads = threads;
    }
    threads = -1;
    cfg.getInt(section, "OperationsThreads", threads);
    if (threads >= 0)
    {
        settings.operation_threads = threads;
    }

    cfg.getBool(section, "DisableCullNode", settings.disable_culling_node);
    cfg.getBool(section, "DisableNativeGLTF", settings.disable_native_gltf_loader);
    cfg.getBool(section, "DrawModelsTwoSided", settings.draw_models_two_sided);
    cfg.getDouble(section, "LargeScaleTileSize", settings.culling_tiles_size_0);
    cfg.getDouble(section, "SmallScaleTileSize", settings.culling_tiles_size_1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadStationsTextSettings(CfgReader& cfg, const QString& section)
{
    double fontSize = 0.0;
    cfg.getDouble(section, "StationsTextFontSize", fontSize);
    if (fontSize > 0.0)
    {
        settings.stations_text_font_size = fontSize;
    }

    QString shift = "0.0 0.0 15.0";
    if (cfg.getString(section, "StationsTextShift", shift))
    {
        std::istringstream stream(shift.toStdString());
        stream >> settings.stations_text_shift.x
            >> settings.stations_text_shift.y
            >> settings.stations_text_shift.z;
    }

    double scaleDistance = -1.0;
    cfg.getDouble(section, "StationsTextScaleDistance", scaleDistance);
    if (scaleDistance >= 0.0)
    {
        settings.stations_text_scale_distance = scaleDistance;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadHUDSettings(CfgReader& cfg, const QString& section)
{
    auto readColor = [&](const QString& key, vsg::vec4& color)
    {
        QString value = QString("%1 %2 %3 %4").arg(color.r).arg(color.g).arg(color.b).arg(color.a);
        if (cfg.getString(section, key, value))
        {
            std::istringstream stream(value.toStdString());
            stream >> color.r >> color.g >> color.b >> color.a;
        }
    };

    readColor("HUDBackground",          settings.hud_background);
    readColor("HUDText",                settings.hud_text);
    readColor("HUDButtonOff",           settings.hud_button_off);
    readColor("HUDButtonOn",            settings.hud_button_on);
    readColor("HUDButtonHovered",       settings.hud_button_hovered);
    readColor("HUDButtonInactive",      settings.hud_button_inactive);
    readColor("HUDButtonInactiveText",  settings.hud_button_inactive_text);

    readColor("HUDCurrentTrain",        settings.hud_current_train);
    readColor("HUDControlledTrain",     settings.hud_controlled_train);
    readColor("HUDWarningText",         settings.hud_warning_text);

    readColor("HUDTimetableDelay",      settings.hud_timetable_delay);
    readColor("HUDTimetablePast",       settings.hud_timetable_past);
    readColor("HUDTimetableCurrent",    settings.hud_timetable_current);
    readColor("HUDTimetableFuture",     settings.hud_timetable_future);

    readColor("HUDTrainProfileGrid",              settings.hud_train_profile_grid);
    readColor("HUDTrainProfileGridLabel",         settings.hud_train_profile_grid_label);
    readColor("HUDTrainProfileBaseline",          settings.hud_train_profile_baseline);
    readColor("HUDTrainProfileCurve",             settings.hud_train_profile_curve);
    readColor("HUDTrainProfileUncontrolled",      settings.hud_train_profile_uncontrolled);
    readColor("HUDTrainProfileCurrent",           settings.hud_train_profile_current);
    readColor("HUDTrainProfileControlled",        settings.hud_train_profile_controlled);
    readColor("HUDTrainProfileStationText",       settings.hud_train_profile_station_text);
    readColor("HUDTrainProfileMast",              settings.hud_train_profile_mast);
    readColor("HUDTrainProfileSignalLetter",      settings.hud_train_profile_signal_letter);
    readColor("HUDTrainProfileSpeedLimitBorder",  settings.hud_train_profile_speed_limit_border);
    readColor("HUDTrainProfileSpeedLimitFill",    settings.hud_train_profile_speed_limit_fill);
    readColor("HUDTrainProfileSpeedLimitText",    settings.hud_train_profile_speed_limit_text);
    readColor("HUDTrainProfileSpeedLimitBg",      settings.hud_train_profile_speed_limit_bg);
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

    cfg.getInt(section, "PhysicalDevice", settings.physical_device);

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
    cfg.getInt(section, "depthFormat", settings.depthFormat);

    cfg.getInt(section, "MaxFPS", settings.max_fps);

    cfg.getBool(section, "EnableDebugLayer", settings.enableDebugLayer);
    cfg.getBool(section, "EnableDebugUtils", settings.enableDebugUtils);

    cfg.getDouble(section, "GPUAllocatedMemoryLimit", settings.allocatedMemoryLimit);
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
    cfg.getInt(section, "NumLights", settings.num_lights);

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
    cfg.getDouble(section, "FreeCamRotMouse", settings.free_cam_rotate_mouse);
    cfg.getDouble(section, "FreeCamHeightStep", settings.free_cam_height_step);

    double freeCamFovYCoeff = 1.0;
    cfg.getDouble(section, "FreeCamFovYCoeff", freeCamFovYCoeff);
    if (freeCamFovYCoeff > 1.01)
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
    cfg.getDouble(section, "FollowCamShiftForward", settings.follow_cam_init_shift_forward);
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
// Парсер командной строки
//------------------------------------------------------------------------------
void RouteViewer::overrideSettingsByCommandLine(int argc, char* argv[])
{
    vsg::CommandLine arguments(&argc, argv);

    // В парсере VSG нет встроенного help, напишем справку об опциях вручную
    if (arguments.read({"-?", "-h", "--help"}))
    {
        std::cout << "Usage: viewer [options]"  << std::endl << std::endl;

        std::cout << "Options:" << std::endl;
        std::cout << "  -?, -h, --help                  Displays help on commandline options"   << std::endl;
        std::cout << "  -a, -host, --host-address <ip>  Host address, default: 127.0.0.1"       << std::endl;
        std::cout << "  -p, --port <port>               Port, default: 1992"                    << std::endl;
        exit(0);
    }

    std::string host_address = settings.tcp_config.host_addr.toStdString();
    uint16_t port = settings.tcp_config.port;
    if (arguments.read({"-a", "-host", "--host-address"}, host_address))
        settings.tcp_config.host_addr = host_address.c_str();
    if (arguments.read({"-p", "--port"}, port))
        settings.tcp_config.port = port;
}
