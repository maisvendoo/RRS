#include "RouteViewer.h"

#include "ConfigReader.h"
#include "filesystem.h"
#include "settings.h"
#include "Logger.h"

#include <sstream>

#include <string>

RouteViewer::RouteViewer(int argc, char* argv[])
    : is_ready(false)
    , settings()
{
    if (init(argc, argv))
    {
        LOG_INFO("Viewer is initialized succesfully");
        is_ready = true;
    }
    else
    {
        LOG_FATAL("Fail to initialize viewer");
    }
}

bool RouteViewer::isReady() const
{
    return true;
}

int RouteViewer::run()
{
    return 0;
}

bool RouteViewer::init(int argc, char* argv[])
{
    FileSystem& fs = FileSystem::getInstance();

    Logger::instance().openFile(fs.getLogsDir() + fs.separator() + "viewer.log");

    // osgDB::DatabasePager *dp = viewer.getDatabasePager();
    // dp->setDoPreCompile(true);
    // dp->setTargetMaximumNumberOfPageLOD(1000);

    loadSettings(fs.getConfigDir() + fs.separator() + "settings.xml");
    LOG_INFO("Loaded settings from settings.xml");

    LogLevel level = LOG_LEVEL_INFO;

    if (settings.notify_level == "INFO")
    {
        level = LOG_LEVEL_INFO;
    }
    else if (settings.notify_level == "WARN")
    {
        level = LOG_LEVEL_WARN;
    }
    else if (settings.notify_level == "FATAL")
    {
        level = LOG_LEVEL_FATAL;
    }

    // vsg::Logger::instance()->level = level;

    auto tf = [&](std::ostream& out) {
        out << "Hio";
    };

    return true;
}

void RouteViewer::loadSettings(const std::string& cfg_path)
{
    try
    {
        ConfigReader cfg(cfg_path);

        cfg.setSection("Client");

        std::string host_addr;
        cfg.getValue("HostAddr", host_addr);
        settings.tcp_config.host_addr = host_addr.c_str();

        cfg.getValue("port", settings.tcp_config.port);

        LOG_INFO("Host for client from settings: %s:%u", host_addr.c_str(), settings.tcp_config.port);

        cfg.getValue("ReconnectInterval", settings.tcp_config.reconnect_interval);
        cfg.getValue("VehiclesPosUpdateInterval", settings.vehicles_pos_update_interval);
        cfg.getValue("VehiclesStateUpdateInterval", settings.vehicles_state_update_interval);
        cfg.getValue("VehicleControlledUpdateInterval", settings.vehicle_controled_update_interval);
        cfg.getValue("ClientDelay", settings.client_delay);

        cfg.setSection("Viewer");

        cfg.getValue("Width", settings.width);
        cfg.getValue("Height", settings.height);
        cfg.getValue("FullScreen", settings.fullscreen);
        cfg.getValue("VSync", settings.vsync);
        cfg.getValue("posX", settings.x);
        cfg.getValue("posY", settings.y);
        cfg.getValue("FovY", settings.fovy);
        cfg.getValue("zNear", settings.zNear);
        cfg.getValue("zFar", settings.zFar);
        cfg.getValue("ScreenNumber", settings.screen_number);
        cfg.getValue("WindowDecoration", settings.window_decoration);
        cfg.getValue("DoubleBuffer", settings.double_buffer);
        cfg.getValue("Samples", settings.samples);
        cfg.getValue("MotionBlur", settings.persistence);
        cfg.getValue("NotifyLevel", settings.notify_level);
        cfg.getValue("ViewDistance", settings.view_distance);

        cfg.getValue("CabineCamRotCoeff", settings.cabine_cam_rot_coeff);
        cfg.getValue("CabineCamFovYStep", settings.cabine_cam_fovy_step);
        cfg.getValue("CabineCamSpeed", settings.cabine_cam_speed);

        cfg.getValue("ExtCamInitDist", settings.ext_cam_init_dist);
        cfg.getValue("ExtCamInitHeight", settings.ext_cam_init_height);
        cfg.getValue("ExtCamInitShift", settings.ext_cam_init_shift);
        cfg.getValue("ExtCamRotCoeff", settings.ext_cam_rot_coeff);
        cfg.getValue("ExtCamSpeed", settings.ext_cam_speed);
        cfg.getValue("ExtCamSpeedCoeff", settings.ext_cam_speed_coeff);
        cfg.getValue("ExtCamMinDist", settings.ext_cam_min_dist);
        cfg.getValue("ExtCamInitAngleH", settings.ext_cam_init_angle_H);
        cfg.getValue("ExtCamInitAngleV", settings.ext_cam_init_angle_V);

        std::string free_cam_init_pos;
        cfg.getValue("FreeCamInitPos", free_cam_init_pos);
        if (!free_cam_init_pos.empty())
        {
            std::istringstream stream(free_cam_init_pos);
            stream >> settings.free_cam_init_pos.x
                >> settings.free_cam_init_pos.y
                >> settings.free_cam_init_pos.z;
        }

        cfg.getValue("FreeCamRotCoeff", settings.free_cam_rot_coeff);
        cfg.getValue("FreeCamSpeed", settings.free_cam_speed);
        cfg.getValue("FreeCamSpeedCoeff", settings.free_cam_speed_coeff);
        cfg.getValue("FreeCamFovY", settings.free_cam_fovy_step);

        cfg.getValue("StatCamDist", settings.stat_cam_dist);
        cfg.getValue("StatCamHeight", settings.stat_cam_height);
        cfg.getValue("StatCamShift", settings.stat_cam_shift);

        cfg.getValue("FrameDiv", settings.interval);
    }
    catch (...)
    {
    }
}
