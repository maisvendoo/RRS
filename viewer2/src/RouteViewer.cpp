#include "RouteViewer.h"

#include "filesystem.h"
#include "CfgReader.h"
#include "Logger.h"
#include "ScreenshotWriter.h"
#include "TrafficLightsHandler.h"
#include "VehiclesHandler.h"
#include "UpdateViewerHandler.h"
#include "UpdateSoundManagerHandler.h"
#include "UpdateStatisticsHandler.h"
#include "Route.h"
#include "RouteLoader.h"

#include "sound-manager.h"
#include "tcp-client.h"

#include <cstdlib>
#include <string>

#include <QApplication>

#include <vsg/app/CommandGraph.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/io/Options.h>
#include <vsg/io/write.h>
#include <vsg/maths/vec3.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsgXchange/all.h>

#include <vsg/app/CloseHandler.h>
#include <vsg/lighting/HardShadows.h>
#include <vsg/maths/sphere.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/threading/OperationThreads.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/utils/SharedObjects.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <vsgImGui/imgui.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include <vulkan/vulkan_core.h>

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

RouteViewer::~RouteViewer()
{
    delete vehicles_handler;
    delete traffic_lights_handler;
    delete screenshot_writer;
    delete sound_manager;
    delete tcp_client;
}

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
    while (!is_ready)
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
    loadSettings();
    LOG_INFO("Loaded settings from settings.xml");

    configureLogLevel();

    LOG_INFO("Override settings from command line");
    overrideSettingsByCommandLine(argc, argv);

    tcp_client = new TcpClient(this);
    LOG_INFO("Created TcpClient");

    sound_manager = new SoundManager();
    LOG_INFO("Created SoundManager");

    screenshot_writer = new ScreenshotWriter("screenshot.png");

    traffic_lights_handler = new TrafficLightsHandler(settings);
    vehicles_handler = new VehiclesHandler(settings, sound_manager);

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

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadSettings()
{
    FileSystem& fs = FileSystem::getInstance();
    const std::string cfg_path = fs.getConfigDir() + fs.separator() + "settings.xml";

    CfgReader cfg;
    if (cfg.load(cfg_path.c_str()))
    {
        QString section = "Client";
        loadNetworkSettings(cfg, section);

        section = "Viewer";
        loadLoggerSettings(cfg, section);
        loadWindowSettings(cfg, section);
        loadCameraSettings(cfg, section);
        loadFreeCameraSettings(cfg, section);
        loadCabineCameraSettings(cfg, section);
        loadExternalCameraSettings(cfg, section);
        loadFollowCameraSettings(cfg, section);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::configureLogLevel()
{
    const std::map<std::string, LogLevel> levels = {
        {"INFO", LOG_LEVEL_INFO},
        {"WARN", LOG_LEVEL_WARN},
        {"FATAL", LOG_LEVEL_FATAL}
    };

    const auto found = levels.find(settings.notify_level);
    Logger::instance().level = (found != levels.end()) ? found->second : LOG_LEVEL_INFO;
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
    options->add(vsgXchange::all::create());

    // Отключаем автоматическое создание узла CullNode в загружаемых моделях
    bool culling = false;
    options->setValue("culling", culling);
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

    std::uint32_t vulkan_version;
    vkEnumerateInstanceVersion(&vulkan_version);

    windowTraits = vsg::WindowTraits::create();
    windowTraits->x = settings.x;
    windowTraits->y = settings.y;
    windowTraits->width = settings.width;
    windowTraits->height = settings.height;
    windowTraits->vulkanVersion = vulkan_version;
    // windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; ???
    // windowTraits->swapchainPreferences.imageCount = 3;
    windowTraits->screenNum = settings.screen_number;
    windowTraits->fullscreen = settings.fullscreen;
    windowTraits->windowTitle = settings.name;
    windowTraits->decoration = settings.window_decoration;
    windowTraits->samples = samples_bit_flag(settings.samples);
    windowTraits->debugLayer = true;
    windowTraits->debugUtils = true;

    auto deviceFeatures = windowTraits->deviceFeatures = vsg::DeviceFeatures::create();
    deviceFeatures->get().samplerAnisotropy = VK_TRUE;
    deviceFeatures->get().depthClamp = VK_TRUE;
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
    catch (const vsg::Exception& exception)
    {
        if ((exception.result == VK_ERROR_INVALID_EXTERNAL_HANDLE) && try_screenNum_exception)
        {
            LOG_WARN(exception.message.c_str());
            LOG_WARN("Try to use default display...");
            windowTraits->screenNum = -1;
            initWindow(false);
        }
        else
        {
            LOG_FATAL(exception.message.c_str());
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
    const double windowWidth = static_cast<double>(window->extent2D().width);
    const double windowHeight = static_cast<double>(window->extent2D().height);
    const double aspectRatio = windowWidth / windowHeight;

    auto perspective = vsg::Perspective::create(
        settings.fovy,
        aspectRatio,
        settings.zNear,
        settings.zFar
    );

    const vsg::dvec3 route_start_point(0.0, 750.0, 0.0);
    const vsg::dvec3 eye = route_start_point + settings.free_cam_start;
    const vsg::dvec3 center = eye + vsg::dvec3(0.0, 1.0, 0.0);

    lookAt = vsg::LookAt::create(eye, center, vsg::dvec3(0.0, 0.0, 1.0));

    camera = vsg::Camera::create(perspective, lookAt, vsg::ViewportState::create(window->extent2D()));
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
    vsg::ref_ptr<vsg::ShaderSet> flat_shader;
    vsg::ref_ptr<vsg::ShaderSet> pbr_shader;
    vsg::ref_ptr<vsg::ShaderSet> phong_shader;

    // Загружаем свои шейдеры
    FileSystem& fs = FileSystem::getInstance();
    std::string shaders_dir_path = fs.getDataDir() + fs.separator() + "shaders";

    std::string flat_path = shaders_dir_path + fs.separator() + "standard_flat_shaded.spv";
    vsg::ref_ptr<vsg::ShaderStage> flat_shader_stage =
        vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", flat_path, options);
    if (flat_shader_stage)
    {
        LOG_INFO("Loaded flat shader: %s", flat_path.c_str());
        flat_shader = vsg::ShaderSet::create();
        flat_shader->stages.push_back(flat_shader_stage);
    }
    else
    {
        LOG_WARN("Fail to load flat shader: %s", flat_path.c_str());
        LOG_INFO("Using default flat shader");
        flat_shader = vsg::createFlatShadedShaderSet(options);
    }

    std::string pbr_path = shaders_dir_path + fs.separator() + "standard_pbr.spv";
    vsg::ref_ptr<vsg::ShaderStage> pbr_shader_stage =
        vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", pbr_path, options);
    if (pbr_shader_stage)
    {
        LOG_INFO("Loaded PBR shader: %s", pbr_path.c_str());
        pbr_shader = vsg::ShaderSet::create();
        pbr_shader->stages.push_back(pbr_shader_stage);
    }
    else
    {
        LOG_WARN("Fail to load PBR shader: %s", pbr_path.c_str());
        LOG_INFO("Using default PBR shader");
        pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(options);
    }

    std::string phong_path = shaders_dir_path + fs.separator() + "standard_phong.spv";
    vsg::ref_ptr<vsg::ShaderStage> phong_shader_stage =
        vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", phong_path);
    if (phong_shader_stage)
    {
        LOG_INFO("Loaded Phong shader: %s", phong_path.c_str());
        phong_shader = vsg::ShaderSet::create();
        phong_shader->stages.push_back(phong_shader_stage);
    }
    else
    {
        LOG_WARN("Fail to load Phong shader: %s", phong_path.c_str());
        LOG_INFO("Using default Phong shader");
        phong_shader = vsg::createPhongShaderSet(options);
    }

    // Можем по своему настроить стадии графического конвейера
    vsg::ref_ptr<vsg::VertexInputState> vertexInputState = vsg::VertexInputState::create();
    vsg::ref_ptr<vsg::InputAssemblyState> inputAssemblyState = vsg::InputAssemblyState::create();
    vsg::ref_ptr<vsg::RasterizationState> rasterizationState = vsg::RasterizationState::create();
    vsg::ref_ptr<vsg::ColorBlendState> colorBlendState = vsg::ColorBlendState::create();
    vsg::ref_ptr<vsg::DepthStencilState> depthStencilState = vsg::DepthStencilState::create();
    vsg::ref_ptr<vsg::MultisampleState> multisampleState = vsg::MultisampleState::create();

    rasterizationState->cullMode = VK_CULL_MODE_NONE;

    colorBlendState->attachments = {
        {
            true,                               // blending enabled
            VK_BLEND_FACTOR_SRC_ALPHA,          // srcColorBlendFactor
            VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,// dstColorBlendFactor
            VK_BLEND_OP_ADD,                    // colorBlendOp
            VK_BLEND_FACTOR_ONE,                // srcAlphaBlendFactor
            VK_BLEND_FACTOR_ZERO,               // dstAlphaBlendFactor
            VK_BLEND_OP_ADD,                    // alphaBlendOp
            VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT
        }
    };

    vsg::GraphicsPipelineStates defaultGraphicsPipelineStates =
    {   vertexInputState,
        inputAssemblyState,
        rasterizationState,
        colorBlendState,
        depthStencilState,
        multisampleState };

    flat_shader->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;
    pbr_shader->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;
    phong_shader->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;

#if 0
        // запись шейдеров в файл
        std::string file;
        file = shaders_dir_path + fs.separator() + "~standard_flat.vsgt";
        vsg::write(flat_shader, file, options);
        file = shaders_dir_path + fs.separator() + "~standard_pbr.vsgt";
        vsg::write(pbr_shader, file, options);
        file = shaders_dir_path + fs.separator() + "~standard_phong.vsgt";
        vsg::write(phong_shader, file, options);
        file = shaders_dir_path + fs.separator() + "~default_flat.vsgt";
        vsg::ref_ptr<vsg::ShaderSet> flat_default = vsg::createFlatShadedShaderSet(options);
        vsg::write(flat_default, file, options);
        file = shaders_dir_path + fs.separator() + "~default_pbr.vsgt";
        vsg::ref_ptr<vsg::ShaderSet> pbr_default = vsg::createPhysicsBasedRenderingShaderSet(options);
        vsg::write(pbr_default, file, options);
        file = shaders_dir_path + fs.separator() + "~default_phong.vsgt";
        vsg::ref_ptr<vsg::ShaderSet> phong_default = vsg::createPhongShaderSet(options);
        vsg::write(phong_default, file, options);
        exit(0);
#endif

    // Добавляем шейдеры в стандартные опции
    options->shaderSets.clear();
    options->shaderSets["flat"] = flat_shader;
    options->shaderSets["pbr"] = pbr_shader;
    options->shaderSets["phong"] = phong_shader;

    // Если у теней включена дальности прорисовки, настраиваем каскад теней
    if (settings.shadow_distance > 0.1)
    {
        auto countNumShadowMaps = [](double dist) -> std::uint32_t
        {
            if (dist > 256.0) return 3;
            if (dist > 64.0) return 2;
            return 1;
        };

        if (settings.shadow_distance > 1000.0)
        {
            settings.shadow_distance = 1000.0;
        }

        const std::uint32_t numShadowMapsPerLight = countNumShadowMaps(settings.shadow_distance);
        shadowSettings = vsg::HardShadows::create(numShadowMapsPerLight);
    }

    // Настраиваем область отрисовки теней
    shadow_region = vsg::RegionOfInterest::create();
    shadow_region->points.resize(5);
    root->addChild(shadow_region);

    // Настраиваем общее освещение
    ambient = vsg::AmbientLight::create();
    ambient->color = vsg::vec3(1.0f, 1.0f, 1.0f);
    ambient->intensity = 0.1f;
    root->addChild(ambient);

    // Настраиваем солнечное освещение
    sun = vsg::DirectionalLight::create();
    sun->color = vsg::vec3(1.0f, 1.0f, 1.0f);
    sun->intensity = 1.0f;
    sun->direction = vsg::normalize(vsg::vec3(1.0f, 1.0f, -1.0f));
    sun->shadowSettings = shadowSettings;
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
    view->viewDependentState->shadowMapBias = shadowMapBias;
    view->viewDependentState->lambda = lambda;
    view->addChild(root);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initCommandGraph()
{
    auto renderGraph = vsg::RenderGraph::create(window, view);

    GUIparams = GUIParams::create();
    auto renderImGui = vsgImGui::RenderImGui::create(window, MyGui::create(GUIparams, options));
    renderGraph->addChild(renderImGui);

    commandGraph = vsg::CommandGraph::create(window, renderGraph);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initViewer()
{
    viewer = vsg::Viewer::create();

    viewer->addWindow(window);

    auto upd_server_control = UpdateControlToServerHandler::create(tcp_client);

    upd_viewer_handler = UpdateViewerHandler::create(
        upd_server_control,
        camera,
        shadow_region,
        screenshot_writer,
        traffic_lights_handler,
        vehicles_handler,
        settings
    );

    auto upd_soundmanager_handler = UpdateSoundManagerHandler::create(camera, sound_manager);
    auto upd_statistis_handler = UpdateStatisticsHandler::create();

    auto close_viewer_handler = vsg::CloseHandler::create(viewer);
    close_viewer_handler->closeKey = vsg::KEY_Undefined;

    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(upd_server_control);
    viewer->addEventHandler(upd_viewer_handler);
    viewer->addEventHandler(upd_soundmanager_handler);
    viewer->addEventHandler(upd_statistis_handler);
    viewer->addEventHandler(close_viewer_handler);

    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});
    viewer->compile();

    options->operationThreads = vsg::OperationThreads::create(4, viewer->status); // ???

    GUIparams->viewer = viewer;
    GUIparams->vehicles_handler = vehicles_handler;
    GUIparams->statistics_handler = upd_statistis_handler.get();
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
    const std::string route_dir_path = fs.combinePath(fs.getRouteRootDir(), settings.route_dir_name);
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

        const std::string model_filename_path = route_dir_path + found_it->second;
        if (!vsg::fileExists(model_filename_path))
        {
            LOG_WARN("Fail to find file: %s", model_filename_path.c_str());
            continue;
        }

        auto pagedLOD = vsg::PagedLOD::create();
        pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), settings.view_distance);
        pagedLOD->filename = model_filename_path;
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
/*
        // Нужен ли depthSorted? Нет, не нужен!
        auto depthSorted = vsg::DepthSorted::create();
        depthSorted->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), 1.0);
        depthSorted->child = pagedLOD;

        matrix->addChild(depthSorted);
*/
        matrix->addChild(pagedLOD);
        //GUIParams::nodes.emplace_back(matrix);

        root->addChild(matrix);
    }

    route.object_ref.clear();
    route.transforms.clear();

    viewer->update();

    auto resourceHints = vsg::ResourceHints::create();
    resourceHints->numDatabasePagerReadThreads = 1;
    resourceHints->shadowMapSize = {4096, 4096}; // 2048 по умолчанию
    viewer->compile(resourceHints);

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
    settings.route_dir_name = route_info.route_dir_name.toStdString();
    // settings.route_dir_name = route_info.route_dir_name.toStdString() + "-gltf";
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

    is_signals = traffic_lights_handler->load(sig_data, settings, viewer, options);

    root->addChild(traffic_lights_handler->getNode());

    connect(tcp_client, &TcpClient::updateSignal,
            traffic_lights_handler, &TrafficLightsHandler::slotUpdateSignal);
/*
    viewer->update();
    viewer->compile();
*/
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

    GUIparams->status = QString("Загрузка подвижного состава...");

    is_vehicles = vehicles_handler->load(data, settings, viewer, options);

    GUIparams->status = QString("");

    if (!is_vehicles)
        return;

    connect(tcp_client, &TcpClient::setVehiclesPositions,
            vehicles_handler, &VehiclesHandler::slotGetVehiclesPosData, Qt::DirectConnection);

    connect(tcp_client, &TcpClient::setVehiclesData,
            vehicles_handler, &VehiclesHandler::slotGetVehiclesStateData, Qt::DirectConnection);

    connect(tcp_client, &TcpClient::setVehicleControlled,
            vehicles_handler, &VehiclesHandler::slotGetVehicleControlled, Qt::DirectConnection);

    connect(vehicles_handler, &VehiclesHandler::updated,
            this, &RouteViewer::slotUpdated, Qt::DirectConnection);

    root->addChild(vehicles_handler->getExterior());
/*
    viewer->update();
    viewer->compile();
*/
    LOG_INFO("Send request for continuous vehicles update");
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE, static_cast<double>(settings.vehicles_pos_update_interval) * 0.001);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_STATE_UPDATE, static_cast<double>(settings.vehicles_state_update_interval) * 0.001);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLE_CONTROLLED_UPDATE, static_cast<double>(settings.vehicle_controled_update_interval) * 0.001);

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
