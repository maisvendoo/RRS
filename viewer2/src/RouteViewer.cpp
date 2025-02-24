#include "RouteViewer.h"

#include "CLI11.hpp"
#include "Route.h"
#include "RouteLoader.h"
#include "TrafficLightsHandler.h"
#include "simulator-info-struct.h"
#include "CfgReader.h"
#include "SoundManager.h"
#include "TrainExteriorHandler.h"
#include "cmd-line.h"
#include "filesystem.h"
#include "network-data-types.h"
#include "settings.h"
#include "Logger.h"
#include "tcp-client.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <QApplication>
#include <QObject>
#include <sstream>

#include <string>
#include <vsg/all.h>
#include <vsg/app/Viewer.h>
#include <vsg/io/read.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/HardShadows.h>
#include <vsg/lighting/PercentageCloserSoftShadows.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/RegionOfInterest.h>
#include <vsg/utils/ShaderSet.h>
#include <vsgXchange/all.h>

RouteViewer::RouteViewer(int argc, char* argv[], QObject* parent)
    : QObject(parent)
    , is_ready(false)
    , is_route(false)
    , tcp_client(new TcpClient(this))
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

RouteViewer::~RouteViewer() = default;

bool RouteViewer::isReady() const
{
    return true;
}

int RouteViewer::run()
{
    auto last_time = std::chrono::system_clock::now();
    while (viewer->advanceToNextFrame())
    {
        QApplication::processEvents();
        // constexpr double sideway_distance = 10.0;
        // constexpr double forward_distance = 150.0;
        // constexpr double min_z = -10.0;
        // constexpr double max_z = 100.0;
        // constexpr double angle = vsg::radians(90.0f);
        // auto dir = lookAt->center - lookAt->eye;
        // dir.z = 0.0;
        // dir = vsg::normalize(dir);
        // vsg::dvec3 left_sideway_dir;
        // left_sideway_dir.x = dir.x * std::cos(angle) - dir.y * std::sin(angle);
        // left_sideway_dir.y = dir.x * std::sin(angle) + dir.y * std::cos(angle);
        // left_sideway_dir.z = 0.0;
        // vsg::dvec3 right_sideway_dir = -left_sideway_dir;
        // auto eye = lookAt->eye;
        // eye.z = 0.0;
        // shadow_region->points[0] = eye + left_sideway_dir * sideway_distance + vsg::dvec3(0.0, 0.0, min_z);
        // shadow_region->points[1] = eye + right_sideway_dir * sideway_distance + vsg::dvec3(0.0, 0.0, min_z);
        // shadow_region->points[2] = eye + dir * forward_distance + right_sideway_dir * sideway_distance + vsg::dvec3(0.0, 0.0, min_z);
        // shadow_region->points[3] = eye + dir * forward_distance + left_sideway_dir * sideway_distance + vsg::dvec3(0.0, 0.0, min_z);
        // shadow_region->points[4] = eye + left_sideway_dir * sideway_distance + vsg::dvec3(0.0, 0.0, max_z);
        // shadow_region->points[5] = eye + right_sideway_dir * sideway_distance + vsg::dvec3(0.0, 0.0, max_z);
        // shadow_region->points[6] = eye + dir * forward_distance + right_sideway_dir * sideway_distance + vsg::dvec3(0.0, 0.0, max_z);
        // shadow_region->points[7] = eye + dir * forward_distance + left_sideway_dir * sideway_distance + vsg::dvec3(0.0, 0.0, max_z);

        viewer->handleEvents();
        viewer->update();
        viewer->recordAndSubmit();
        viewer->present();

        auto current_time = std::chrono::system_clock::now();
        auto delta_time = current_time - last_time;
        last_time = current_time;
        double delta = std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();
        // LOG_INFO("FPS: %f", 1.0 / delta * 1000.0);

        // LOG_INFO("%f %f %f", lookAt->eye.x, lookAt->eye.y, lookAt->eye.z);
    }

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

    Logger::instance().level = level;

    LOG_INFO("Override settings from command line");
    overrideSettingsByCommandLine(argc, argv);

    sound_manager = std::make_unique<SoundManager>();
    LOG_INFO("Created SoundManager");

    train_ext_handler = std::make_unique<TrainExteriorHandler>(settings, sound_manager);

    initVsgOptions();
    initWindowTraits();
    initWindow();
    initCamera();
    initScenegraph();
    initLights();
    initView();
    initCommandGraph();
    initViewer();

    initTCPclient();

    // TODO: Скриншоты

    return true;
}

void RouteViewer::loadSettings(const std::string& cfg_path)
{
    CfgReader cfg;
    if (cfg.load(cfg_path.c_str()))
    {
        QString secName = "Client";
        cfg.getString(secName, "HostAddr", settings.tcp_config.host_addr);

        int tmp_int = 0;
        if (cfg.getInt(secName, "port", tmp_int))
        {
            settings.tcp_config.port = static_cast<quint16>(tmp_int);
        }

        cfg.getInt(secName, "ReconnectInteval", settings.tcp_config.reconnect_interval);
        cfg.getInt(secName, "VehiclesPosUpdateInterval", settings.vehicles_pos_update_interval);
        cfg.getInt(secName, "VehiclesStateUpdateInterval", settings.vehicles_state_update_interval);
        cfg.getInt(secName, "VehicleControlledUpdateInterval", settings.vehicle_controled_update_interval);
        cfg.getInt(secName, "ClientDelay", settings.client_delay);

        secName = "Viewer";

        cfg.getInt(secName, "Width", settings.width);
        cfg.getInt(secName, "Height", settings.height);
        cfg.getBool(secName, "FullScreen", settings.fullscreen);
        cfg.getBool(secName, "VSync", settings.vsync);
        cfg.getInt(secName, "posX", settings.x);
        cfg.getInt(secName, "posY", settings.y);
        cfg.getDouble(secName, "FovY", settings.fovy);
        cfg.getDouble(secName, "zNear", settings.zNear);
        cfg.getDouble(secName, "zFar", settings.zFar);

        tmp_int = 0;
        cfg.getInt(secName, "ScreenNumber", tmp_int);
        if (tmp_int >= 0)
            settings.screen_number = tmp_int;

        cfg.getBool(secName, "WindowDecoration", settings.window_decoration);
        cfg.getBool(secName, "DoubleBuffer", settings.double_buffer);
        cfg.getBool(secName, "Samples", settings.samples);
        cfg.getDouble(secName, "MotionBlur", settings.persistence);
        cfg.getDouble(secName, "ViewDistance", settings.view_distance);

        QString tmp_qstr = "INFO";
        if (cfg.getString(secName, "NotifyLevel", tmp_qstr))
        {
            settings.notify_level = tmp_qstr.toStdString();
        }

        cfg.getDouble(secName, "CabineCamRotCoeff", settings.cabine_cam_rot_coeff);
        cfg.getDouble(secName, "CabineCamFovYStep", settings.cabine_cam_fovy_step);
        cfg.getDouble(secName, "CabineCamSpeed", settings.cabine_cam_speed);

        cfg.getDouble(secName, "ExtCamInitDist", settings.ext_cam_init_dist);
        cfg.getDouble(secName, "ExtCamInitHeight", settings.ext_cam_init_height);
        cfg.getDouble(secName, "ExtCamInitShift", settings.ext_cam_init_shift);
        cfg.getDouble(secName, "ExtCamRotCoeff", settings.ext_cam_rot_coeff);
        cfg.getDouble(secName, "ExtCamSpeed", settings.ext_cam_speed);
        cfg.getDouble(secName, "ExtCamSpeedCoeff", settings.ext_cam_speed_coeff);
        cfg.getDouble(secName, "ExtCamMinDist", settings.ext_cam_min_dist);
        cfg.getDouble(secName, "ExtCamInitAngleH", settings.ext_cam_init_angle_H);
        cfg.getDouble(secName, "ExtCamInitAngleV", settings.ext_cam_init_angle_V);

        tmp_qstr = "0.0 0.0 0.0";
        if (cfg.getString(secName, "FreeCamInitPos", tmp_qstr))
        {
            std::string free_cam_init_pos = tmp_qstr.toStdString();
            std::istringstream stream(free_cam_init_pos);
            stream >> settings.free_cam_init_pos.x
                >> settings.free_cam_init_pos.y
                >> settings.free_cam_init_pos.z;
        }

        cfg.getDouble(secName, "FreeCamRotCoeff", settings.free_cam_rot_coeff);
        cfg.getDouble(secName, "FreeCamSpeed", settings.free_cam_speed);
        cfg.getDouble(secName, "FreeCamSpeedCoeff", settings.free_cam_speed_coeff);
        cfg.getDouble(secName, "FreeCamFovY", settings.free_cam_fovy_step);

        cfg.getDouble(secName, "StatCamDist", settings.stat_cam_dist);
        cfg.getDouble(secName, "StatCamHeight", settings.stat_cam_height);
        cfg.getDouble(secName, "StatCamShift", settings.stat_cam_shift);

        tmp_int = 0;
        cfg.getInt(secName, "FrameDiv", tmp_int);
        if (tmp_int > 0)
            settings.interval = tmp_int;
    }
}

int RouteViewer::overrideSettingsByCommandLine(int argc, char* argv[])
{
    cmd_line_t cmd_line;

    CLI::App app("Viewer");
    argv = app.ensure_utf8(argv);

    app.add_option("--host-addr", cmd_line.host_addr);
    app.add_option("--port", cmd_line.port);
    app.add_option("--width", cmd_line.width);
    app.add_option("--height", cmd_line.height);
    app.add_flag("--fullscreen", cmd_line.fullscreen);
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

    settings.fullscreen = cmd_line.fullscreen;

    if (cmd_line.notify_level)
    {
        settings.notify_level = cmd_line.notify_level.value();
    }

    return 0;
}

void RouteViewer::initVsgOptions()
{
    options = vsg::Options::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->sharedObjects = vsg::SharedObjects::create();
    options->add(vsgXchange::all::create());
}

void RouteViewer::initWindowTraits()
{
    windowTraits = vsg::WindowTraits::create();
    windowTraits->x = settings.x;
    windowTraits->y = settings.y;
    windowTraits->width = settings.width;
    windowTraits->height = settings.height;
    windowTraits->windowTitle = settings.name;
    windowTraits->decoration = settings.window_decoration;
    windowTraits->samples = settings.samples;
    // windowTraits->debugLayer = true;
    // windowTraits->debugUtils = true;
}

void RouteViewer::initWindow()
{
    window = vsg::Window::create(windowTraits);
}

void RouteViewer::initCamera()
{
    constexpr vsg::dvec3 center(0.0, 1100.0, 0.0);
    constexpr double radius = 100.0;
    constexpr double nearFarRatio = 0.001;

    double windowWidth = static_cast<double>(window->extent2D().width);
    double windowHeight = static_cast<double>(window->extent2D().height);
    double aspectRatio = windowWidth / windowHeight;

    auto perspective = vsg::Perspective::create(settings.fovy, aspectRatio, nearFarRatio * radius, radius * 4.5);

    vsg::dvec3 eye = center + vsg::dvec3(0.0, -radius * 3.5, 20.0);
    lookAt = vsg::LookAt::create(eye, center, vsg::dvec3(0.0, 0.0, 1.0));

    camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));
}

void RouteViewer::initScenegraph()
{
    root = vsg::Group::create();
}

void RouteViewer::initLights()
{
    auto deviceFeatures = windowTraits->deviceFeatures = vsg::DeviceFeatures::create();
        deviceFeatures->get().samplerAnisotropy = VK_TRUE;
        deviceFeatures->get().depthClamp = VK_TRUE;

    auto numShadowMapsPerLight = 3;
    // shadowSettings = vsg::PercentageCloserSoftShadows::create(numShadowMapsPerLight);

    auto shaderHints = vsg::ShaderCompileSettings::create();

    float penumbraRadius = 0.005f;
    shadowSettings = vsg::HardShadows::create(numShadowMapsPerLight);

    auto rasterizationState = vsg::RasterizationState::create();
    rasterizationState->depthClampEnable = VK_TRUE;

    auto pbr = options->shaderSets["pbr"] = vsg::createPhysicsBasedRenderingShaderSet(options);
    pbr->defaultGraphicsPipelineStates.push_back(rasterizationState);
    pbr->defaultShaderHints = shaderHints;
    pbr->variants.clear();

    auto phong = options->shaderSets["phong"] = vsg::createPhongShaderSet(options);
    phong->defaultGraphicsPipelineStates.push_back(rasterizationState);
    phong->defaultShaderHints = shaderHints;
    phong->variants.clear();

    auto flat = options->shaderSets["flat"] = vsg::createPhysicsBasedRenderingShaderSet(options);
    flat->defaultGraphicsPipelineStates.push_back(rasterizationState);
    flat->defaultShaderHints = shaderHints;
    flat->variants.clear();

    // shadow_region = vsg::RegionOfInterest::create();
    // shadow_region->points.emplace_back();
    // shadow_region->points.emplace_back();
    // shadow_region->points.emplace_back();
    // shadow_region->points.emplace_back();
    // shadow_region->points.emplace_back();
    // shadow_region->points.emplace_back();
    // shadow_region->points.emplace_back();
    // shadow_region->points.emplace_back();

    // root->addChild(shadow_region);

    auto ambient = vsg::AmbientLight::create();
    ambient->color = vsg::vec3(1.0f, 1.0f, 1.0f);
    ambient->intensity = 0.1f;

    sun = vsg::DirectionalLight::create();
    sun->color = vsg::vec3(1.0f, 1.0f, 1.0f);
    sun->intensity = 1.0f;
    sun->direction = vsg::normalize(vsg::vec3(1.0f, 1.0f, -1.0f));
    sun->shadowSettings = shadowSettings;

    root->addChild(ambient);
    root->addChild(sun);
}

void RouteViewer::initView()
{
    constexpr double maxShadowDistance = 1e8;
    constexpr double shadowMapBias = 0.005;
    constexpr double lambda = 0.25;

    view = vsg::View::create();
    view->camera = camera;
    view->viewDependentState->maxShadowDistance = maxShadowDistance;
    // view->viewDependentState->shadowMapBias = shadowMapBias;
    view->viewDependentState->lambda = lambda;
    // view->viewDependentState->shadowSettingsOverride[{}] = vsg::HardShadows::create(1);
    view->addChild(root);
    auto renderGraph = vsg::RenderGraph::create(window, view);
    commandGraph = vsg::CommandGraph::create(window, renderGraph);
}

void RouteViewer::initCommandGraph()
{

}

void RouteViewer::initViewer()
{
    viewer = vsg::Viewer::create();
    viewer->addWindow(window);

    viewer->addEventHandler(vsg::CloseHandler::create(viewer));
    viewer->addEventHandler(vsg::Trackball::create(camera));

    // auto commandGraph = vsg::createCommandGraphForView(window, camera, root);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});
    viewer->compile();
}

void RouteViewer::initTCPclient()
{
    LOG_INFO("Starting init TCP-client");

    connect(tcp_client, &TcpClient::connected, this, &RouteViewer::slotConnectedToSimulator);
    connect(tcp_client, &TcpClient::setRouteInfo, this, &RouteViewer::slotGetRouteInfoData);
    connect(tcp_client, &TcpClient::setSignalsData, this, &RouteViewer::slotGetSignalsData);
    connect(tcp_client, &TcpClient::setVehiclesInfo, this, &RouteViewer::slotGetVehicleInfoData);
    connect(tcp_client, &TcpClient::sendLogMessage, this, &RouteViewer::slotRecvLogMessage);

    tcp_client->init(settings.tcp_config);

    LOG_INFO("TCP-client is initialized...OK");
}

bool RouteViewer::loadRoute()
{
    if (settings.route_dir_name.empty())
    {
        LOG_ERROR("Route directory name is empty");
        return false;
    }

    FileSystem& fs = FileSystem::getInstance();
    std::string route_dir_path = fs.combinePath(fs.getRouteRootDir(), settings.route_dir_name);
    settings.route_dir_full_path = route_dir_path;

    Route route;

    RouteLoader loader(settings.route_dir_full_path);
    loader.read_description();
    loader.parse_objects_ref(route);
    loader.parse_route_map(route);

    for (auto& [label, transform] : route.transforms)
    {
        auto found_it = route.object_ref.find(label);
        if (found_it == route.object_ref.end())
        {
            continue;
        }

        auto pagedLOD = vsg::PagedLOD::create();
        pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), 200.0);
        pagedLOD->filename = route_dir_path + found_it->second;
        pagedLOD->options = options;

        auto matrix = vsg::MatrixTransform::create();
        transform.r_x = -vsg::radians(transform.r_x);
        transform.r_y = -vsg::radians(transform.r_y);
        transform.r_z = -vsg::radians(transform.r_z);

        auto rotate_x = vsg::rotate(transform.r_x, vsg::vec3(1.0f, 0.0f, 0.0f));
        auto rotate_y = vsg::rotate(transform.r_y, vsg::vec3(0.0f, 1.0f, 0.0f));
        auto rotate_z = vsg::rotate(transform.r_z, vsg::vec3(0.0f, 0.0f, 1.0f));
        auto translate = vsg::translate(transform.t_x, transform.t_y, transform.t_z);
        matrix->matrix = translate * rotate_z * rotate_y * rotate_x;

        matrix->addChild(pagedLOD);

        root->addChild(matrix);
    }

    viewer->update();
    viewer->compile();

    return true;
}

void RouteViewer::slotRecvLogMessage(QString msg)
{
    LOG_INFO("%s", msg.toStdString().c_str());
}

void RouteViewer::slotConnectedToSimulator()
{
    LOG_INFO("Connected to server...OK");
    LOG_INFO("Send request for route info");
    tcp_client->sendRequest(STYPE_REQUEST_ROUTE_INFO);
}

void RouteViewer::slotGetRouteInfoData(QByteArray &data)
{
    if (is_route)
    {
        LOG_WARN("Get route info again");
        return;
    }
    is_route = true;

    /*
    QString msg = QString("Загрузка маршрута...");
    imguiWidgetsHandler->setLoadingStatus(msg);
    */

    simulator_route_info_t route_info;
    route_info.deserialize(data);
    settings.route_dir_name = route_info.route_dir_name.toStdString()/* + "-gltf"*/;
    LOG_INFO("Get route directory name: %s", settings.route_dir_name.c_str());

    loadRoute();

    // FileSystem& fs = FileSystem::getInstance();
    // auto test_node = vsg::read_cast<vsg::Node>(fs.getRouteRootDir() + "/experimental-polygon-gltf/models/floorl.gltf", options);
    // std::cout << "\n\n\n" << test_node.get() << "\n\n\n";

    // viewer->update();
    // viewer->compile();

    LOG_INFO("Send request for signals data");
    tcp_client->sendRequest(STYPE_REQUEST_SIGNALS_DATA);
}

void RouteViewer::slotGetSignalsData(QByteArray &sig_data)
{
    if (is_signals)
    {
        LOG_WARN("Get signals data again");
        return;
    }
    is_signals = true;

    traffic_lights_handler->deserialize(sig_data);

    traffic_lights_handler->create_pagedLODs(settings, options);
    traffic_lights_handler->loadSignalModels(settings, options, shadowSettings);
    root->addChild(traffic_lights_handler->traffic_light_nodes);

    connect(tcp_client, &TcpClient::updateSignal,
            traffic_lights_handler.get(), &TrafficLightsHandler::slotUpdateSignal);

    traffic_lights_update_handler = TrafficLightsUpdateHandler::create(traffic_lights_handler.get());

    viewer->addEventHandler(traffic_lights_update_handler);

    viewer->update();
    viewer->compile();
}

void RouteViewer::slotGetVehicleInfoData(QByteArray &data)
{

}

void RouteViewer::slotUpdateKeyboard()
{

}

void RouteViewer::slotUpdateControlledVehicle()
{

}

