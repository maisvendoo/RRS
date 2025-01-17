#include "RouteViewer.h"

#include "ConfigReader.h"
#include "filesystem.h"
#include "settings.h"

#include <sstream>
#include <vsg/io/Logger.h>

#include <string>

#include <cstdint>

RouteViewer::RouteViewer(int argc, char* argv[])
    : is_ready(false)
    , settings()
{
    if (init(argc, argv))
    {
        vsg::Logger::instance()->info("Viewer is initialized succesfully");
        is_ready = true;
    }
    else
    {
        vsg::Logger::instance()->fatal("Fail to initialize viewer");
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

    // osgDB::DatabasePager *dp = viewer.getDatabasePager();
    // dp->setDoPreCompile(true);
    // dp->setTargetMaximumNumberOfPageLOD(1000);

    loadSettings(fs.getConfigDir() + fs.separator() + "settings.xml");

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

        vsg::Logger::instance()->info(
            std::string("Host for client from settings: ")
            + host_addr
            + ':'
            + std::to_string(settings.tcp_config.port)
        );

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
            stream >> settings.free_cam_init_pos;
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
