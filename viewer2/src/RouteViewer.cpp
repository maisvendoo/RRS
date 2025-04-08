#include "RouteViewer.h"

#include "cmake_defines.h"
#include "cmd-line.h"
#include "CLI11.hpp"
#include "filesystem.h"
#include "CfgReader.h"
#include "Logger.h"
#include "ScreenshotWriter.h"
#include "TrafficLightsHandler.h"
#include "VehiclesHandler.h"
#include "UpdateViewerHandler.h"
#include "UpdateSoundManagerHandler.h"
#include "Route.h"
#include "RouteLoader.h"

#include "simulator-info-struct.h"
#include "sound-manager.h"
#include "tcp-client.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <QApplication>

#include <vsgXchange/all.h>

#include <vsg/app/CloseHandler.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/HardShadows.h>
#include <vsg/lighting/PercentageCloserSoftShadows.h>
#include <vsg/maths/sphere.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/utils/SharedObjects.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <vsgImGui/imgui.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>

RouteViewer::RouteViewer(int argc, char* argv[], QObject* parent) : QObject(parent)
{
    if (init(argc, argv))
    {
        LOG_INFO("Viewer is initialized succesfully");
        //is_ready = true;
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int RouteViewer::run()
{
    // Обрабатываем события сетевой подсистемы, дожидаемся загрузки и
    // инициализации все объектов
    while (!isReady())
    {
        QApplication::processEvents();
    }

    // Главный цикл рендеринга
    while (viewer->advanceToNextFrame())
    {
        QApplication::processEvents();

        viewer->handleEvents();
        viewer->update();

        if (screenshot_writer->isScreeenshot())
            screenshot_writer->doScreeenshot(window, options);

        viewer->recordAndSubmit();
        viewer->present();
    }

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::init(int argc, char* argv[])
{
    FileSystem& fs = FileSystem::getInstance();

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

    tcp_client = new TcpClient(this);
    LOG_INFO("Created TcpClient");

    sound_manager = new SoundManager();
    LOG_INFO("Created SoundManager");
    
    initVsgOptions();
    
    screenshot_writer = new ScreenshotWriter("screenshot.png");

    traffic_lights_handler = new TrafficLightsHandler(nullptr, options);

    vehicles_handler = new VehiclesHandler(settings, sound_manager);

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadSettings(const std::string& cfg_path)
{
    CfgReader cfg;
    if (cfg.load(cfg_path.c_str()))
    {
        QString secName = "Client";
        cfg.getString(secName, "HostAddr", settings.tcp_config.host_addr);

        int tmp_int = 0;
        if (cfg.getInt(secName, "port", tmp_int))
            settings.tcp_config.port = static_cast<quint16>(tmp_int);

        cfg.getInt(secName, "ReconnectInteval", settings.tcp_config.reconnect_interval);
        cfg.getInt(secName, "VehiclesPosUpdateInterval", settings.vehicles_pos_update_interval);
        cfg.getInt(secName, "VehiclesStateUpdateInterval", settings.vehicles_state_update_interval);
        cfg.getInt(secName, "VehicleControlledUpdateInterval", settings.vehicle_controled_update_interval);
        cfg.getInt(secName, "ClientDelay", settings.client_delay);

        secName = "Viewer";

        QString tmp_qstr = "INFO";
        if (cfg.getString(secName, "NotifyLevel", tmp_qstr))
            settings.notify_level = tmp_qstr.toStdString();

        tmp_qstr = "viewer";
        if (cfg.getString(secName, "Name", tmp_qstr))
            settings.name = tmp_qstr.toStdString();

        cfg.getInt(secName, "posX", settings.x);
        cfg.getInt(secName, "posY", settings.y);
        cfg.getInt(secName, "Width", settings.width);
        cfg.getInt(secName, "Height", settings.height);
        tmp_int = 0;
        cfg.getInt(secName, "ScreenNumber", tmp_int);
        if (tmp_int >= 0) settings.screen_number = tmp_int;
        cfg.getBool(secName, "FullScreen", settings.fullscreen);
        cfg.getBool(secName, "VSync", settings.vsync);
        cfg.getBool(secName, "WindowDecoration", settings.window_decoration);

        cfg.getBool(secName, "DoubleBuffer", settings.double_buffer);
        cfg.getInt(secName, "Samples", settings.samples);

        cfg.getDouble(secName, "ViewDistance", settings.view_distance);
        cfg.getDouble(secName, "zNear", settings.zNear);
        cfg.getDouble(secName, "zFar", settings.zFar);
        cfg.getDouble(secName, "FovY", settings.fovy);
        cfg.getDouble(secName, "FovYMin", settings.fovy_min);
        cfg.getDouble(secName, "FovYMax", settings.fovy_max);
        cfg.getDouble(secName, "PitchMin", settings.pitch_min);
        cfg.getDouble(secName, "PitchMax", settings.pitch_max);

        // Положение свободной камеры при запуске
        tmp_qstr = "0.0 0.0 0.0";
        if (cfg.getString(secName, "FreeCamStart", tmp_qstr))
        {
            std::string free_cam_start = tmp_qstr.toStdString();
            std::istringstream stream(free_cam_start);
            stream >> settings.free_cam_start.x
                >> settings.free_cam_start.y
                >> settings.free_cam_start.z;
        }
        // Настройки свободной камеры
        tmp_qstr = "0.0 0.0 0.0";
        if (cfg.getString(secName, "FreeCamInitPos", tmp_qstr))
        {
            std::string free_cam_init_pos = tmp_qstr.toStdString();
            std::istringstream stream(free_cam_init_pos);
            stream >> settings.free_cam_init_pos.x
                >> settings.free_cam_init_pos.y
                >> settings.free_cam_init_pos.z;
        }
        cfg.getDouble(secName, "FreeCamSpeedKeyboard", settings.free_cam_speed_keyboard);
        cfg.getDouble(secName, "FreeCamSpeedMouse", settings.free_cam_speed_mouse);
        double tmp_double = 1.0;
        cfg.getDouble(secName, "FreeCamSpeedCoeff", tmp_double);
        if (tmp_double > 1.01) settings.free_cam_speed_coeff = tmp_double;
        cfg.getDouble(secName, "FreeCamRotKeyboard", settings.free_cam_rotate_keyboard);
        cfg.getDouble(secName, "FreeCamRotMouse", settings.free_cam_rotate_keyboard);
        cfg.getDouble(secName, "FreeCamHeightStep", settings.free_cam_height_step);
        tmp_double = 1.0;
        cfg.getDouble(secName, "FreeCamFovYCoeff", tmp_double);
        if (tmp_double > 1.01) settings.free_cam_fovy_coeff = tmp_double;

        // Настройки камеры в кабине
        tmp_qstr = "0.0 0.0 0.0";
        if (cfg.getString(secName, "CabineCamInitPos", tmp_qstr))
        {
            std::string free_cam_init_pos = tmp_qstr.toStdString();
            std::istringstream stream(free_cam_init_pos);
            stream >> settings.cabine_default_pos.x
                >> settings.cabine_default_pos.y
                >> settings.cabine_default_pos.z;
        }
        cfg.getDouble(secName, "CabineCamSpeedKeyboard", settings.cabine_speed_keyboard);
        cfg.getDouble(secName, "CabineCamSpeedMouse", settings.cabine_speed_mouse);
        tmp_double = 1.0;
        cfg.getDouble(secName, "CabineCamSpeedCoeff", tmp_double);
        if (tmp_double > 1.01) settings.cabine_speed_coeff = tmp_double;
        cfg.getDouble(secName, "CabineCamRotKeyboard", settings.cabine_rotate_keyboard);
        cfg.getDouble(secName, "CabineCamRotMouse", settings.cabine_rotate_mouse);
        cfg.getDouble(secName, "CabineCamHeightStep", settings.cabine_height_step);
        tmp_double = 1.0;
        cfg.getDouble(secName, "CabineCamFovYCoeff", tmp_double);
        if (tmp_double > 1.01) settings.cabine_fovy_coeff = tmp_double;
        cfg.getDouble(secName, "CabineCamVerticalShiftMin", settings.cabine_z_min);
        cfg.getDouble(secName, "CabineCamVerticalShiftMax", settings.cabine_z_max);

        // Настройки внешней камеры
        tmp_qstr = "0.0 0.0 0.0";
        if (cfg.getString(secName, "ExtCamInitPos", tmp_qstr))
        {
            std::string free_cam_init_pos = tmp_qstr.toStdString();
            std::istringstream stream(free_cam_init_pos);
            stream >> settings.ext_cam_init_pos.x
                >> settings.ext_cam_init_pos.y
                >> settings.ext_cam_init_pos.z;
        }
        cfg.getDouble(secName, "ExtCamInitAngleH", settings.ext_cam_init_angle_H);
        cfg.getDouble(secName, "ExtCamInitAngleV", settings.ext_cam_init_angle_V);
        cfg.getDouble(secName, "ExtCamInitDist", settings.ext_cam_init_distance);
        cfg.getDouble(secName, "ExtCamSpeedKeyboard", settings.ext_cam_speed_keyboard);
        cfg.getDouble(secName, "ExtCamSpeedMouse", settings.ext_cam_speed_mouse);
        tmp_double = 1.0;
        cfg.getDouble(secName, "ExtCamSpeedCoeff", tmp_double);
        if (tmp_double > 1.01) settings.ext_cam_speed_coeff = tmp_double;
        cfg.getDouble(secName, "ExtCamRotKeyboard", settings.ext_cam_rotate_keyboard);
        cfg.getDouble(secName, "ExtCamRotMouse", settings.ext_cam_rotate_mouse);
        cfg.getDouble(secName, "ExtCamHeightStep", settings.ext_cam_height_step);
        tmp_double = 1.0;
        cfg.getDouble(secName, "ExtCamDistCoeff", tmp_double);
        if (tmp_double > 1.01) settings.ext_cam_dist_coeff = tmp_double;
        cfg.getDouble(secName, "ExtCamDistMin", settings.ext_cam_dist_min);

        // Настройки следящей камеры
        cfg.getDouble(secName, "FollowCamShiftRight", settings.follow_cam_init_shift_right);
        cfg.getDouble(secName, "FollowCamShiftUp", settings.follow_cam_init_shift_up);
        cfg.getDouble(secName, "FollowCamFwdVelocityCoeff", settings.follow_cam_fwd_velocity_coeff);
        cfg.getDouble(secName, "FollowCamSpeedKeyboard", settings.follow_cam_speed_keyboard);
        cfg.getDouble(secName, "FollowCamSpeedMouse", settings.follow_cam_speed_mouse);
        tmp_double = 1.0;
        cfg.getDouble(secName, "FollowCamSpeedCoeff", tmp_double);
        if (tmp_double > 1.01) settings.follow_cam_speed_coeff = tmp_double;
        tmp_double = 1.0;
        cfg.getDouble(secName, "FollowCamFovYCoeff", tmp_double);
        if (tmp_double > 1.01) settings.follow_cam_fovy_coeff = tmp_double;
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initVsgOptions()
{
    options = vsg::Options::create();
    options->fileCache = vsg::getEnv("VSG_FILE_CACHE");
    options->paths = vsg::getEnvPaths("VSG_FILE_PATH");
    options->sharedObjects = vsg::SharedObjects::create();
    options->propagateDynamicObjects = vsg::PropagateDynamicObjects::create();
    options->add(vsgXchange::all::create());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initWindowTraits()
{
    auto samples_bit_flag = [](int s) -> VkSampleCountFlags
    {
        if (s > 63) return VK_SAMPLE_COUNT_64_BIT;
        if (s > 31) return VK_SAMPLE_COUNT_32_BIT;
        if (s > 15) return VK_SAMPLE_COUNT_16_BIT;
        if (s > 7) return VK_SAMPLE_COUNT_8_BIT;
        if (s > 3) return VK_SAMPLE_COUNT_4_BIT;
        if (s > 1) return VK_SAMPLE_COUNT_2_BIT;
        return VK_SAMPLE_COUNT_1_BIT;
    };
    windowTraits = vsg::WindowTraits::create();
    windowTraits->x = settings.x;
    windowTraits->y = settings.y;
    windowTraits->width = settings.width;
    windowTraits->height = settings.height;
    windowTraits->screenNum = settings.screen_number;
    windowTraits->fullscreen = settings.fullscreen;
    windowTraits->windowTitle = settings.name;
    windowTraits->decoration = settings.window_decoration;
    windowTraits->samples = samples_bit_flag(settings.samples);
    // windowTraits->debugLayer = true;
    // windowTraits->debugUtils = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initWindow(bool try_screenNum_exception)
{
    try
    {
        window = vsg::Window::create(windowTraits);
    }
    catch (const vsg::Exception e)
    {
        if ((e.result == VK_ERROR_INVALID_EXTERNAL_HANDLE) && try_screenNum_exception)
        {
            LOG_WARN(e.message.c_str());
            LOG_WARN("Try to use default display...");
            windowTraits->screenNum = -1;
            initWindow(false);
        }
        else
        {
            LOG_FATAL(e.message.c_str());
            LOG_FATAL("Fail to create window");
            exit(1);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initCamera()
{
    double windowWidth = static_cast<double>(window->extent2D().width);
    double windowHeight = static_cast<double>(window->extent2D().height);
    double aspectRatio = windowWidth / windowHeight;

    auto perspective = vsg::Perspective::create(settings.fovy,
        aspectRatio, settings.zNear, settings.zFar);

    vsg::dvec3 route_start_point(0.0, 750.0, 0.0);
    vsg::dvec3 eye = route_start_point + settings.free_cam_start;
    vsg::dvec3 center = eye + vsg::dvec3(0.0, 1.0, 0.0);

    lookAt = vsg::LookAt::create(eye, center, vsg::dvec3(0.0, 0.0, 1.0));

    camera = vsg::Camera::create(perspective, lookAt,
        vsg::ViewportState::create(window->extent2D()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initScenegraph()
{
    root = vsg::Group::create();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initLights()
{
    auto deviceFeatures = windowTraits->deviceFeatures = vsg::DeviceFeatures::create();
    deviceFeatures->get().samplerAnisotropy = VK_TRUE;
    deviceFeatures->get().depthClamp = VK_TRUE;

    auto numShadowMapsPerLight = 1;
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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

    GUIparams = GUIParams::create();
    auto renderImGui = vsgImGui::RenderImGui::create(window, MyGui::create(GUIparams, options));
    renderGraph->addChild(renderImGui);

    commandGraph = vsg::CommandGraph::create(window, renderGraph);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initCommandGraph()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initViewer()
{
    viewer = vsg::Viewer::create();
    GUIparams->viewer = viewer;
    GUIparams->vehicles_handler = vehicles_handler;

    viewer->addWindow(window);

    auto upd_server_control = UpdateControlToServerHandler::create(tcp_client);
    upd_viewer_handler = UpdateViewerHandler::create(
        upd_server_control, camera, screenshot_writer, traffic_lights_handler, vehicles_handler, settings);
    auto close_viewer_handler = vsg::CloseHandler::create(viewer);
    close_viewer_handler->closeKey = vsg::KEY_Undefined;

    viewer->addEventHandler(upd_server_control);
    viewer->addEventHandler(upd_viewer_handler);
    viewer->addEventHandler(UpdateSoundManagerHandler::create(camera, sound_manager));
    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(close_viewer_handler);

    // auto commandGraph = vsg::createCommandGraphForView(window, camera, root);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});
    viewer->compile();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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
        //GUIParams::nodes.emplace_back(matrix);

        root->addChild(matrix);
    }

    viewer->update();
    viewer->compile();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotRecvLogMessage(QString msg)
{
    LOG_INFO("%s", msg.toStdString().c_str());
    GUIparams->status = msg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotConnectedToSimulator()
{
    LOG_INFO("Connected to server...OK");
    LOG_INFO("Send request for route info");
    tcp_client->sendRequest(STYPE_REQUEST_ROUTE_INFO);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotGetRouteInfoData(QByteArray &data)
{
    if (is_route)
    {
        LOG_WARN("Get route info again");
        return;
    }
    is_route = true;

    GUIparams->status = QString("Загрузка маршрута...");

    simulator_route_info_t route_info;
    route_info.deserialize(data);
    settings.route_dir_name = route_info.route_dir_name.toStdString() + "-gltf";
    LOG_INFO("Get route directory name: %s", settings.route_dir_name.c_str());

    loadRoute();

    LOG_INFO("Send request for signals data");
    tcp_client->sendRequest(STYPE_REQUEST_SIGNALS_DATA);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotGetSignalsData(QByteArray &sig_data)
{
    LOG_INFO("Got signals data from server");
    if (is_signals)
    {
        LOG_WARN("Get signals data again");
        return;
    }
    is_signals = true;

    GUIparams->status = QString("Загрузка светофоров...");

    traffic_lights_handler->deserialize(sig_data);

    traffic_lights_handler->create_pagedLODs(settings);
    traffic_lights_handler->loadSignalModels(settings, shadowSettings);
    root->addChild(traffic_lights_handler->traffic_light_nodes);

    connect(tcp_client, &TcpClient::updateSignal,
            traffic_lights_handler, &TrafficLightsHandler::slotUpdateSignal);

    viewer->update();
    viewer->compile();

    LOG_INFO("Send request for vehicles info");
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_INFO);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotGetVehicleInfoData(QByteArray &data)
{
    if (is_vehicles)
    {
        LOG_WARN("Get vehicles info again");
        return;
    }
    is_vehicles = true;

    simulator_vehicles_info_t vehicles_info;
    vehicles_info.deserialize(data);
    int count = vehicles_info.vehicles.size();
    if (count <= 0)
    {
        LOG_WARN("Server has not any vehicles");
        is_vehicles = false;
        return;
    }

    LOG_INFO("Get info about %u vehicles", count);

    GUIparams->status = QString("Загрузка подвижного состава...");

    vehicles_handler->load(vehicles_info, settings, options);

    GUIparams->status = QString("");

    connect(tcp_client, &TcpClient::setVehiclesPositions,
            vehicles_handler, &VehiclesHandler::slotGetVehiclesPosData, Qt::DirectConnection);

    connect(tcp_client, &TcpClient::setVehiclesData,
            vehicles_handler, &VehiclesHandler::slotGetVehiclesStateData, Qt::DirectConnection);

    connect(tcp_client, &TcpClient::setVehicleControlled,
            vehicles_handler, &VehiclesHandler::slotGetVehicleControlled, Qt::DirectConnection);

    connect(vehicles_handler, &VehiclesHandler::updated,
            this, &RouteViewer::slotUpdated, Qt::DirectConnection);

    root->addChild(vehicles_handler->getExterior());

    viewer->update();
    viewer->compile();

    LOG_INFO("Send request for continuous vehicles update");
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE,
                            static_cast<double>(settings.vehicles_pos_update_interval) / 1000.0);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_STATE_UPDATE,
                            static_cast<double>(settings.vehicles_state_update_interval) / 1000.0);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLE_CONTROLLED_UPDATE,
                            static_cast<double>(settings.vehicle_controled_update_interval) / 1000.0);

    is_ready = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotUpdated()
{
    // Камера в кабину ПЕ через фиктивное нажатие F1
    vsg::KeyPressEvent keyPress(window, viewer->getFrameStamp()->time, vsg::KEY_F1, vsg::KEY_F1, vsg::MODKEY_OFF);
    upd_viewer_handler->apply(keyPress);
}
