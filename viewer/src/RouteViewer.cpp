#include "RouteViewer.h"

#include "AnimatedDatabasePager.h"
#include "CfgReader.h"
#include "Logger.h"
#include "MyGui.h"
#include "NewSkybox.h"
#include "Route.h"
#include "RouteLoader.h"
#include "ScreenshotWriter.h"
// #include "Skybox.h"
#include "Sun.h"
#include "TrafficLightsHandler.h"
#include "UpdateLightingHandler.h"
#include "UpdateControlToServerHandler.h"
#include "UpdateSoundManagerHandler.h"
#include "UpdateStatisticsHandler.h"
#include "UpdateViewerHandler.h"
#include "VehiclesHandler.h"
#include "filesystem.h"
#include "sound-manager.h"
#include "tcp-client.h"

#include <vsg/app/CloseHandler.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/lighting/HardShadows.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/RegionOfInterest.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/threading/OperationThreads.h>
#include <vsg/utils/ShaderSet.h>
#include <vsg/utils/SharedObjects.h>
#include <vsg/vk/DeviceFeatures.h>
#include <vsgImGui/RenderImGui.h>
#include <vsgImGui/SendEventsToImGui.h>
#include <vsgXchange/all.h>

#include <QApplication>

#include <cstdlib>
#include <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::RouteViewer(QObject* parent)
    : QObject(parent)
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteViewer::~RouteViewer()
{
    delete vehicles_handler;
    delete traffic_lights_handler;
    delete screenshot_writer;
    delete sound_manager;
    delete tcp_client;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initialize(int argc, char* argv[])
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

    screenshot_writer = new ScreenshotWriter("screenshot.jpg");

    traffic_lights_handler = new TrafficLightsHandler();
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

    if (vsg::ref_ptr<vsg::Device> vulkan_device = window->getDevice())
    {
        if (const vsg::PhysicalDevice* phys_device = vulkan_device->getPhysicalDevice())
        {
            const VkPhysicalDeviceProperties& propeties = phys_device->getProperties();
            auto device_type_to_string = [](VkPhysicalDeviceType device_type) -> const char* {
                switch (device_type)
                {
                    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:    return "discrete";
                    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:  return "integrated";
                    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:     return "virtual";
                    case VK_PHYSICAL_DEVICE_TYPE_CPU:             return "CPU";
                    default:                                      return "<other_type>";
                }
            };

            LOG_INFO("Using %s device: %s",
                     device_type_to_string(propeties.deviceType),
                     propeties.deviceName);

            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(*phys_device, &memory_properties);
            for (std::uint32_t heap_idx = 0; heap_idx < VK_MAX_MEMORY_HEAPS; ++heap_idx)
            {
                const std::uint64_t memory_size = memory_properties.memoryHeaps[heap_idx].size;
                if (memory_size > 0)
                {
                    LOG_INFO("Device's memory[%u] size = %u MB", heap_idx, memory_size / 1024 / 1024);
                }
            }
        }
        else
        {
            LOG_WARN("WARN: No physical device");
        }
    }
    else
    {
        LOG_WARN("WARN: No Vulkan device");
    }

    vehicles_handler->set_camera_pos(&lookAt->eye);

    initTcpClient();

    LOG_INFO("Viewer is initialized succesfully");
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

    // viewer->setupThreading(); // Эта функция была в одном из vsgExamples
                                 // Вызывает ошибку на выходе из вьювера

    // Главный цикл рендеринга
    while (viewer->advanceToNextFrame())
    {
        QApplication::processEvents();

        viewer->handleEvents();
        viewer->update();

        if (screenshot_writer->isScreeenshot())
        {
            screenshot_writer->doScreeenshot(window, options);
        }

        viewer->recordAndSubmit();
        viewer->present();
    }

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadSettings()
{
    const FileSystem& fs = FileSystem::getInstance();
    const std::string cfg_path = fs.getConfigDir() + fs.separator() + "settings.xml";

    CfgReader cfg;
    if (cfg.load(cfg_path.c_str()))
    {
        QString section = "Client";
        loadNetworkSettings(cfg, section);

        section = "Viewer";
        loadLoggerSettings(cfg, section);
        loadModelsSettings(cfg, section);
        loadWindowSettings(cfg, section);
        loadLightSettings(cfg, section);
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
void RouteViewer::configureLogLevel() const
{
    if (settings.notify_level == "INFO")
    {
        Logger::instance().level = LOG_LEVEL_INFO;
    }
    else if (settings.notify_level == "WARN")
    {
        Logger::instance().level = LOG_LEVEL_WARN;
    }
    else if (settings.notify_level == "FATAL")
    {
        Logger::instance().level = LOG_LEVEL_FATAL;
    }
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
    options->setValue("culling", !settings.disable_culling_node);

    // Отключение нативного загрузчика .gltf в VSG, чтобы использовать assimp
    options->setValue("disable_gltf", settings.disable_native_gltf_loader);

    GUIparams = GUIParams::create();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initWindowTraits()
{
    auto samples_bit_flag = [](int s) -> VkSampleCountFlags
    {
        if (s > 7)
        {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        else if (s > 3)
        {
            return VK_SAMPLE_COUNT_4_BIT;
        }
        else if (s > 1)
        {
            return VK_SAMPLE_COUNT_2_BIT;
        }
        else
        {
            return VK_SAMPLE_COUNT_1_BIT;
        }
    };

    // std::uint32_t vulkan_version;
    // vkEnumerateInstanceVersion(&vulkan_version);

    windowTraits = vsg::WindowTraits::create();
    windowTraits->x = settings.x;
    windowTraits->y = settings.y;
    windowTraits->width = settings.width;
    windowTraits->height = settings.height;
    // windowTraits->vulkanVersion = vulkan_version; // VSG и так берет самую новую версию
    // windowTraits->swapchainPreferences.imageCount = 3;
    windowTraits->screenNum = settings.screen_number;
    windowTraits->fullscreen = settings.fullscreen;
    windowTraits->windowTitle = settings.name;
    windowTraits->decoration = settings.window_decoration;
    windowTraits->samples = samples_bit_flag(settings.samples);
    // windowTraits->debugLayer = true;
    // windowTraits->debugUtils = true;

    // Настройка вертикальной синхронизации (упрощенно - вкл/выкл)
    windowTraits->swapchainPreferences.presentMode = settings.vsync ? VK_PRESENT_MODE_FIFO_KHR
                                                                    : VK_PRESENT_MODE_IMMEDIATE_KHR;

    // auto deviceFeatures = windowTraits->deviceFeatures = vsg::DeviceFeatures::create(); // VSG и так создает deviceFeatures по умолчанию
    // deviceFeatures->get().samplerAnisotropy = VK_TRUE;                                  // и выставляет samplerAnisotropy в true

    auto deviceFeatures = windowTraits->deviceFeatures;
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
        settings.view_distance
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

    // Модель неба - создаём в первую очередь,
    // до всего остального в сцене и до компиляции вьювера
    FileSystem& fs = FileSystem::getInstance();
    const std::string cfg_path = fs.getConfigDir() + fs.separator() + "skybox.xml";

    // Skybox* skybox = new Skybox(cfg_path, options);
    // GUIparams->skybox = skybox;
    // GUIparams->skybox_texture_data = skybox->getDefaultTexture();
    // GUIparams->skybox_textures = skybox->getTextures();

    // if (skybox->getNode())
    // {
    //     // root->addChild(skybox->getNode());
    // }

    NewSkybox* nsb = new NewSkybox(cfg_path, options);
    GUIparams->new_skybox = nsb;

    if (nsb->get_state_group())
    {
        root->addChild(nsb->get_state_group());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initLights()
{
    configureShaders();

    // Если тени включены, создаём настройки с количеством каскадов
    if (settings.shadow)
    {
        shadowSettings = vsg::HardShadows::create(settings.shadow_cascade);

        // Округляем разрешение карт теней до степени двойки
        // в разумных пределах от 2^8 (256x256) до 2^16 (65536x65536)
        int power_of_two = 8;
        while ((power_of_two < 16)
               && (settings.shadow_resolution > std::pow(2, power_of_two)))
        {
            ++power_of_two;
        }
        settings.shadow_resolution = std::pow(2, power_of_two);
    }

    // Настраиваем область отрисовки теней
    shadow_region = vsg::RegionOfInterest::create();
    shadow_region->points.resize(5);

    root->addChild(shadow_region);

    // Освещение
    sun = Sun::create(lookAt->eye, settings.ambient_intensity, settings.sun_intensity);
    root->addChild(sun);
    GUIparams->sun = sun;

    // Настраиваем общее освещение
    sun->ambient->color = vsg::vec3(settings.ambient_color);

    // Настраиваем солнечное освещение
    sun->sun->color = vsg::vec3(settings.sun_color);
    sun->sun->shadowSettings = shadowSettings;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::configureShaders()
{
    // За основу берём встроенные комплекты вершинного и фрагментного шейдера
    auto flat_shader = vsg::createFlatShadedShaderSet(options);
    auto pbr_shader = vsg::createPhysicsBasedRenderingShaderSet(options);
    auto phong_shader = vsg::createPhongShaderSet(options);

    // Загружаем свои варианты вершинного и фрагментного шейдера вместо встроенного
    FileSystem& fs = FileSystem::getInstance();
    const std::string shaders_dir_path = fs.getDataDir() + fs.separator() + "shaders";

    loadCustomShader(fs, shaders_dir_path, "standard.vert", "standard_flat_shaded.frag", "flat", flat_shader);
    loadCustomShader(fs, shaders_dir_path, "standard.vert", "standard_pbr.frag", "PBR", pbr_shader);
    loadCustomShader(fs, shaders_dir_path, "standard.vert", "standard_phong.frag", "Phong", phong_shader);

    // Можем по своему настроить стадии графического конвейера
    auto vertexInputState = vsg::VertexInputState::create();
    auto inputAssemblyState = vsg::InputAssemblyState::create();
    auto rasterizationState = vsg::RasterizationState::create();
    auto colorBlendState = vsg::ColorBlendState::create();
    auto depthStencilState = vsg::DepthStencilState::create();
    auto multisampleState = vsg::MultisampleState::create();

    // Рисуем текстуры на обеих сторонах полигонов
    if (settings.draw_models_two_sided)
    {
        rasterizationState->cullMode = VK_CULL_MODE_NONE;
    }

    // Включаем отображение объектов за плоскостями отсечения
    // для корректной работы теней от объектов за пределами вида камеры
    if (settings.shadow)
    {
        rasterizationState->depthClampEnable = VK_TRUE;
    }

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

    vsg::GraphicsPipelineStates defaultGraphicsPipelineStates = {
        vertexInputState,
        inputAssemblyState,
        rasterizationState,
        colorBlendState,
        depthStencilState,
        multisampleState
    };

    flat_shader->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;
    pbr_shader->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;
    phong_shader->defaultGraphicsPipelineStates = defaultGraphicsPipelineStates;

#if 0
    // запись шейдеров в файл
        std::string file;

        file = shaders_dir_path + fs.separator() + "~custom_flat.vsgt";
        vsg::write(flat_shader, file, options);

        file = shaders_dir_path + fs.separator() + "~custom_pbr.vsgt";
        vsg::write(pbr_shader, file, options);

        file = shaders_dir_path + fs.separator() + "~custom_phong.vsgt";
        vsg::write(phong_shader, file, options);

        file = shaders_dir_path + fs.separator() + "~default_flat.vsgt";
        vsg::ref_ptr<vsg::ShaderSet> flat_default = vsg::createFlatShadedShaderSet();
        vsg::write(flat_default, file, options);

        file = shaders_dir_path + fs.separator() + "~default_pbr.vsgt";
        vsg::ref_ptr<vsg::ShaderSet> pbr_default = vsg::createPhysicsBasedRenderingShaderSet();
        vsg::write(pbr_default, file, options);

        file = shaders_dir_path + fs.separator() + "~default_phong.vsgt";
        vsg::ref_ptr<vsg::ShaderSet> phong_default = vsg::createPhongShaderSet();
        vsg::write(phong_default, file, options);

        exit(0);
#endif

    // Добавляем шейдеры в стандартные опции
    options->shaderSets.clear();
    options->shaderSets["flat"] = flat_shader;
    options->shaderSets["pbr"] = pbr_shader;
    options->shaderSets["phong"] = phong_shader;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::loadCustomShader(
    FileSystem& fs,
    const std::string& shaders_dir_path,
    const char* vert_shader_filename,
    const char* frag_shader_filename,
    const char* shader_set_name,
    vsg::ref_ptr<vsg::ShaderSet> shader_set
)
{
    auto vert_shader_path = shaders_dir_path + fs.separator() + vert_shader_filename;
    auto vert_shader_stage = vsg::ShaderStage::read(VK_SHADER_STAGE_VERTEX_BIT, "main", vert_shader_path, options);

    auto frag_shader_path = shaders_dir_path + fs.separator() + frag_shader_filename;
    auto frag_shader_stage = vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", frag_shader_path, options);

    if (!vert_shader_stage)
    {
        LOG_WARN("Failed to load vertex shader: %s", vert_shader_path.c_str());
        LOG_INFO("Using default %s shader set", shader_set_name);
        return;
    }

    if (!frag_shader_stage)
    {
        LOG_WARN("Failed to load fragment shader: %s", frag_shader_path.c_str());
        LOG_INFO("Using default %s shader set", shader_set_name);
        return;
    }

    LOG_INFO("Loaded custom %s shader set: %s, %s", shader_set_name, vert_shader_path.c_str(), frag_shader_path.c_str());

    shader_set->stages.front() = vert_shader_stage;
    shader_set->stages.back() = frag_shader_stage;

    // Очищаем все встроенные сохранённые варианты настроек
    shader_set->variants.clear();
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

    auto upd_sound_manager_handler = UpdateSoundManagerHandler::create(lookAt, sound_manager);
    auto upd_statistis_handler = UpdateStatisticsHandler::create();

    auto close_viewer_handler = vsg::CloseHandler::create(viewer);
    close_viewer_handler->closeKey = vsg::KEY_Undefined;

    auto upd_lighting_handler = UpdateLightingHandler::create(sun, vehicles_handler);

    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(upd_server_control);
    viewer->addEventHandler(upd_viewer_handler);
    viewer->addEventHandler(upd_sound_manager_handler);
    viewer->addEventHandler(upd_statistis_handler);
    viewer->addEventHandler(upd_lighting_handler);
    viewer->addEventHandler(close_viewer_handler);

    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

    // Перед компиляцией вьювера подсовываем ему наш кастомный DatabasePager
    vsg::ref_ptr<vsg::DatabasePager> databasePager = AnimatedDatabasePager::create();
    for (auto& task : viewer->recordAndSubmitTasks)
    {
        task->databasePager = databasePager;
    }

    // Перед компиляцией вьювера применяем некоторые настройки
    auto resourceHints = vsg::ResourceHints::create();
    // Указываем грузить модели в один поток, иначе будут дубликаты в памяти
    resourceHints->numDatabasePagerReadThreads = 1;
    // Указываем разрешение карты теней
    resourceHints->shadowMapSize = {static_cast<uint32_t>(settings.shadow_resolution),
                                    static_cast<uint32_t>(settings.shadow_resolution)};
    // Указываем допустимое количество источников света
    resourceHints->numLightsRange = {static_cast<uint32_t>(settings.num_lights),
                                     static_cast<uint32_t>(settings.num_lights + 1)};
    viewer->compile(resourceHints);

    options->operationThreads = vsg::OperationThreads::create(1, viewer->status);

    GUIparams->viewer = viewer;
    GUIparams->vehicles_handler = vehicles_handler;
    GUIparams->statistics_handler = upd_statistis_handler.get();
    GUIparams->controls_handler = upd_server_control.get();

    is_ready = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initTcpClient()
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

    // Загрузка информации о моделях в маршруте
    Route route;

    RouteLoader loader(settings.route_dir_full_path);
    loader.read_description();
    loader.parse_objects_ref(route);
    loader.parse_route_map(route);

    // Создание PagedLOD для моделей в маршруте
    vsg::ref_ptr<vsg::Group> route_root = vsg::Group::create();
    auto current = route.transforms.begin();
    while (current != route.transforms.end())
    {
        std::string label = current->first;
        auto range = route.transforms.equal_range(label);

        auto found_it = route.object_ref.find(label);
        if (found_it == route.object_ref.end())
        {
            current = range.second;
            continue;
        }

        const std::string model_filename_path = route_dir_path + found_it->second;
        if (!vsg::fileExists(model_filename_path))
        {
            LOG_WARN("Fail to find file: %s", model_filename_path.c_str());
            current = range.second;
            continue;
        }

        auto pagedLOD = vsg::PagedLOD::create();
        pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), settings.view_distance);
        pagedLOD->children[0] = vsg::PagedLOD::Child{0.1, {}};
        pagedLOD->filename = model_filename_path;
        pagedLOD->options = options;

        for (auto it = range.first; it != range.second; ++it)
        {
            auto& transform = it->second;

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
            route_root->addChild(matrix);
        }

        current = range.second;
    }

    route.object_ref.clear();
    route.transforms.clear();

    root->addChild(route_root);

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

    is_signals = traffic_lights_handler->load(sig_data, settings.route_dir_full_path, options);

    root->addChild(traffic_lights_handler->getNode());

    connect(tcp_client, &TcpClient::updateSignal,
            traffic_lights_handler, &TrafficLightsHandler::slotUpdateSignal);

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

    is_vehicles = vehicles_handler->load(data, settings, options);

    GUIparams->status = QString("");

    if (!is_vehicles)
    {
        return;
    }

    connect(tcp_client, &TcpClient::setVehiclesPositions,
            vehicles_handler, &VehiclesHandler::slotGetVehiclesPosData, Qt::DirectConnection);

    connect(tcp_client, &TcpClient::setVehiclesData,
            vehicles_handler, &VehiclesHandler::slotGetVehiclesStateData, Qt::DirectConnection);

    connect(tcp_client, &TcpClient::setVehicleControlled,
            vehicles_handler, &VehiclesHandler::slotGetVehicleControlled, Qt::DirectConnection);

    connect(vehicles_handler, &VehiclesHandler::updated,
            this, &RouteViewer::slotUpdated, Qt::DirectConnection);

    root->addChild(vehicles_handler->getExterior());

    LOG_INFO("Send request for continuous vehicles update");
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE, static_cast<double>(settings.vehicles_pos_update_interval) * 0.001);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_STATE_UPDATE, static_cast<double>(settings.vehicles_state_update_interval) * 0.001);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLE_CONTROLLED_UPDATE, static_cast<double>(settings.vehicle_controled_update_interval) * 0.001);
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
