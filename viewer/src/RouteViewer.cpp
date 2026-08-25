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
#include "StationsHandler.h"
#include "Sun.h"
#include "TrafficLightsHandler.h"
#include "UpdateControlToServerHandler.h"
#include "UpdateSoundManagerHandler.h"
#include "UpdateStatisticsHandler.h"
#include "UpdateViewerHandler.h"
#include "VehiclesHandler.h"
#include "WorldCulling.h"
#include "filesystem.h"
#include "graphics/common.h"
#include "sound-manager.h"
#include "tcp-client.h"
#include "graphics/shader_funcs.h"

#include <vsg/app/CloseHandler.h>
#include <vsg/app/Viewer.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/RenderGraph.h>
#include <vsg/app/View.h>
#include <vsg/core/Array.h>
#include <vsg/core/ConstVisitor.h>
#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/lighting/HardShadows.h>
#include <vsg/maths/sphere.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/CullNode.h>
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

#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

#include <AltSoundLocker.h>

#include <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
/// Операция подключения скомпилированного подграфа в сцену.
/// Выполняется в фазе update, чтобы не пересекаться с рендером в других потоках.
struct Merge final : public vsg::Inherit<vsg::Operation, Merge>
{
    Merge(vsg::ref_ptr<vsg::Viewer> in_viewer,
          vsg::ref_ptr<vsg::Group> in_root,
          vsg::ref_ptr<vsg::Node> in_subgraph,
          const vsg::CompileResult& in_compile_result) :
        root(in_root),
        subgraph(in_subgraph),
        compile_result(in_compile_result)
    {
        viewer = in_viewer;
    }

    void run() override
    {
        // Превращаем observer_ptr в ref_ptr, чтобы проверить живость viewer'а
        if (vsg::ref_ptr<vsg::Viewer> ref_viewer = viewer;
            ref_viewer && root && subgraph)
        {
            updateViewer(*ref_viewer, compile_result);

            root->addChild(subgraph);
        }
    }

    vsg::observer_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsg::Group> root;
    vsg::ref_ptr<vsg::Node> subgraph;
    vsg::CompileResult compile_result;
};

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
RouteViewer::~RouteViewer() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::checkPhysicalDeviceProperties()
{
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

    tcp_client = std::make_unique<TcpClient>(this);
    LOG_INFO("Created TcpClient");

    sound_manager = std::make_unique<SoundManager>();
    LOG_INFO("Created SoundManager");

    screenshot_writer = std::make_unique<ScreenshotWriter>("screenshot.jpg");

    traffic_lights_handler = std::make_unique<TrafficLightsHandler>();
    stations_handler = std::make_unique<StationsHandler>(settings);
    vehicles_handler = std::make_unique<VehiclesHandler>(settings, sound_manager.get());

    initVsgOptions();
    initWindowTraits();
    initWindow();
    initCamera();
    initScenegraph();
    initLights();
    initView();
    initCommandGraph();
    initViewer();

    checkPhysicalDeviceProperties();

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
    constexpr int MAX_WAIT_ITERATIONS = 600; // ~60s at 100ms per iteration
    int wait_count = 0;
    while (!is_ready)
    {
        QApplication::processEvents();

        if (is_connection_abandoned)
        {
            LOG_ERROR("Cannot start rendering — no connection to simulator");
            return 1;
        }

        if (++wait_count >= MAX_WAIT_ITERATIONS)
        {
            LOG_ERROR("Timed out waiting for simulator data");
            return 1;
        }
    }

    using clock = std::chrono::steady_clock;
    using namespace std::chrono;

    auto target_frame_time = microseconds(0);
    if (!settings.vsync && settings.max_fps > 0)
    {
        target_frame_time = duration_cast<microseconds>(duration<double>(1.0 / settings.max_fps));
    }

    auto next_frame_time = clock::now();

    while (viewer->advanceToNextFrame())
    {
        try
        {
            // Ждём до точного времени начала кадра (с коррекцией дрифта)
            if (target_frame_time.count() > 0)
            {
                auto now = clock::now();
                if (now < next_frame_time)
                {
                    std::this_thread::sleep_until(next_frame_time);
                }
                else
                {
                    // Пропустили кадр — сбрасываем таймер, чтобы не копить отставание
                    next_frame_time = now;
                }
            }

            QApplication::processEvents();
            viewer->handleEvents();
            viewer->update();

            if (screenshot_writer && screenshot_writer->isScreeenshot())
            {
                screenshot_writer->doScreeenshot(window, options);
            }

            viewer->recordAndSubmit();
            viewer->present();

            // Планируем следующий кадр
            if (target_frame_time.count() > 0)
            {
                next_frame_time += target_frame_time;

                // Защита от спирали отставания
                auto now = clock::now();
                if (next_frame_time < now)
                {
                    next_frame_time = now;
                }
            }
        }
        catch (const vsg::Exception& e)
        {
            LOG_ERROR("Vulkan error in render loop: %s (VkResult %d)", e.message.c_str(), e.result);
            break;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("Exception in render loop: %s", e.what());
            break;
        }
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
        loadStationsTextSettings(cfg, section);
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
    options = create_default_vsg_options();

    // Отключаем автоматическое создание узла CullNode в загружаемых моделях
    options->setValue("culling", !settings.disable_culling_node);

    // Отключение нативного загрузчика .gltf в VSG, чтобы использовать assimp
    options->setValue("disable_gltf", settings.disable_native_gltf_loader);

    GUIparams = GUIParams::create();
    GUIparams->train_profile_backward = settings.train_profile_backward;
    GUIparams->train_profile_forward = settings.train_profile_forward;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VkFormat getDepthFormat(int idx)
{
    switch (idx)
    {
    case 0:
        return VK_FORMAT_D16_UNORM;
    case 1:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case 2:
        return VK_FORMAT_D32_SFLOAT;
    case 3:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    return VK_FORMAT_D16_UNORM;
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

    windowTraits->depthFormat = getDepthFormat(settings.depthFormat);

    windowTraits->debugLayer = settings.enableDebugLayer;
    windowTraits->debugUtils = settings.enableDebugUtils;

    // Настройка вертикальной синхронизации (упрощенно - вкл/выкл)
    // MAILBOX = triple-buffered, no tearing, no stall on missed vsync
    // FIFO = double-buffered, waits for next vsync on miss (causes hitching)
    // IMMEDIATE = no sync at all (tearing)
    if (settings.vsync)
    {
        if (settings.max_fps <= 0)
        {
            windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        else
        {
            windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        }
    }
    else
    {
        windowTraits->swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

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
        // Получаем инстанс Vulkan от созданного окна
        auto instance = window->getOrCreateInstance();
        // ОБЯЗАТЕЛЬНО создаем поверхность рендеринга!!!
        auto surface = window->getOrCreateSurface();
        // Поучаем список физических устройств
        auto physDevs = instance->getPhysicalDevices();

        // Защита от дурака
        if (settings.physical_device < 0 || settings.physical_device > physDevs.size() - 1)
        {
            settings.physical_device = 0;
        }

        auto physDev = physDevs[settings.physical_device];

        auto props = physDev->getProperties();
        GUIparams->physicalDeviceName = QString(props.deviceName);

        // Устанавливаем устройство из настроек
        window->setPhysicalDevice(physDev);

        lockAltSound(window.get());
    }
    catch (const vsg::Exception& exception)
    {
        if ((exception.result == VK_ERROR_INVALID_EXTERNAL_HANDLE) && try_screenNum_exception)
        {
            LOG_WARN(exception.message.c_str());
            LOG_WARN("Try to use default display...");
            windowTraits->screenNum = -1;
            try
            {
                window = vsg::Window::create(windowTraits);
                lockAltSound(window.get());
            }
            catch (const vsg::Exception& e2)
            {
                LOG_FATAL(e2.message.c_str());
                LOG_FATAL("Fail to create window on fallback display");
                exit(1);
            }
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

    world_culling = WorldCulling::create(settings.culling_tiles_size_0, settings.culling_tiles_size_1);
    root->addChild(world_culling->world_root);

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

    skybox = std::make_unique<NewSkybox>(cfg_path, options);
    GUIparams->new_skybox = skybox.get();

    if (auto node = skybox->getNode())
    {
        root->addChild(node);
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

    const auto vertex_shader = read_shader(shaders_dir_path.c_str(), "standard.vert", options);

    configure_shader_set(shaders_dir_path.c_str(), vertex_shader,
        "standard_flat_shaded.frag", options, "flat", flat_shader);
    configure_shader_set(shaders_dir_path.c_str(), vertex_shader,
        "standard_pbr.frag", options, "PBR", pbr_shader);
    configure_shader_set(shaders_dir_path.c_str(), vertex_shader,
        "standard_phong.frag", options, "Phong", phong_shader);

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

    auto upd_server_control = UpdateControlToServerHandler::create(tcp_client.get());

    upd_viewer_handler = UpdateViewerHandler::create(
        upd_server_control,
        camera,
        shadow_region,
        screenshot_writer.get(),
        traffic_lights_handler.get(),
        vehicles_handler.get(),
        settings
    );

    auto upd_sound_manager_handler = UpdateSoundManagerHandler::create(lookAt, sound_manager.get());
    auto upd_statistis_handler = UpdateStatisticsHandler::create();

    auto close_viewer_handler = vsg::CloseHandler::create(viewer);
    close_viewer_handler->closeKey = vsg::KEY_Undefined;

    viewer->addEventHandler(vsgImGui::SendEventsToImGui::create());
    viewer->addEventHandler(upd_server_control);
    viewer->addEventHandler(upd_viewer_handler);
    viewer->addEventHandler(upd_sound_manager_handler);
    viewer->addEventHandler(upd_statistis_handler);
    viewer->addEventHandler(close_viewer_handler);

    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

    // Перед компиляцией вьювера подсовываем ему наш кастомный DatabasePager
    vsg::ref_ptr<AnimatedDatabasePager> databasePager = AnimatedDatabasePager::create();
    databasePager->targetMaxNumPagedLODWithHighResSubgraphs = settings.targetPagedLODs;
    databasePager->cullingScreenHeightRatio = settings.cullingScreenHeightRatio;
    for (auto& task : viewer->recordAndSubmitTasks)
    {
        task->databasePager = databasePager;
    }

    // Перед компиляцией вьювера применяем некоторые настройки
    auto resourceHints = vsg::ResourceHints::create();
    // Указываем количество потоков чтения 3d-моделей
    uint32_t numThreads = std::max(1u, std::thread::hardware_concurrency() / 2);
    uint32_t numReadThreads = std::min(settings.read_threads, numThreads);
    resourceHints->numDatabasePagerReadThreads = numReadThreads;
    // Указываем разрешение карты теней
    resourceHints->shadowMapSize = {static_cast<uint32_t>(settings.shadow_resolution),
                                    static_cast<uint32_t>(settings.shadow_resolution)};
    // Указываем допустимое количество источников света
    resourceHints->numLightsRange = {static_cast<uint32_t>(settings.num_lights),
                                     static_cast<uint32_t>(settings.num_lights + 1)};
    auto compileResult = viewer->compile(resourceHints);
    if (!compileResult)
    {
        LOG_WARN("Viewer compile returned empty result — some resources may not have been compiled");
    }

    // Задаем лимит выделение видеопамяти (для разработчиков! отладка работы на слабых системах!)
    auto device = window->getDevice();
    auto memPolls = device->deviceMemoryBufferPools.ref_ptr();

    if (memPolls)
    {
        memPolls->allocatedMemoryLimit = std::clamp(settings.allocatedMemoryLimit, 0.0, 1.0);
    }

    // Создаём вспомогательные потоки для чтения текстур 3d-моделей
    uint32_t numOpThreads = std::min(settings.operation_threads, numThreads);
    if (numOpThreads)
    {
        options->operationThreads = vsg::OperationThreads::create(numOpThreads, viewer->status);
    }

    GUIparams->viewer = viewer;
    GUIparams->vehicles_handler = vehicles_handler.get();
    GUIparams->viewer_handler = upd_viewer_handler.get();
    GUIparams->statistics_handler = upd_statistis_handler.get();
    GUIparams->controls_handler = upd_server_control.get();
    GUIparams->traffic_lights_handler = traffic_lights_handler.get();
    GUIparams->stations_handler = stations_handler.get();

    is_ready = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initTcpClient()
{
    LOG_INFO("Starting init TCP-client");

    connect(tcp_client.get(), &TcpClient::connected, this, &RouteViewer::slotConnectedToSimulator);
    connect(tcp_client.get(), &TcpClient::setRouteInfo, this, &RouteViewer::slotGetRouteInfoData);
    connect(tcp_client.get(), &TcpClient::setSignalsData, this, &RouteViewer::slotGetSignalsData);
    connect(tcp_client.get(), &TcpClient::setStationsData, this, &RouteViewer::slotGetStationsData);
    connect(tcp_client.get(), &TcpClient::setVehiclesInfo, this, &RouteViewer::slotGetVehicleInfoData);
    connect(tcp_client.get(), &TcpClient::sendLogMessage, this, &RouteViewer::slotRecvLogMessage);
    connect(tcp_client.get(), &TcpClient::connectionAbandoned, this, [this]() {
        LOG_ERROR("Connection to simulator abandoned — exiting viewer");
        is_connection_abandoned = true;
    });

    tcp_client->init(settings.tcp_config);

    GUIparams->tcp_client = tcp_client.get();

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
    for (auto& [label, transforms] : route.route_map)
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

        {
            auto pagedLOD = vsg::PagedLOD::create();
            pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), settings.view_distance);
            pagedLOD->children[0] = vsg::PagedLOD::Child{0.0, {}};
            pagedLOD->filename = model_filename_path;
            pagedLOD->options = options;

            for (auto& transform : transforms)
            {
                vsg::vec3& rotation_deg = transform.rotation_deg;

                auto matrix = vsg::MatrixTransform::create();
                rotation_deg.x = -vsg::radians(rotation_deg.x);
                rotation_deg.y = -vsg::radians(rotation_deg.y);
                rotation_deg.z = -vsg::radians(rotation_deg.z);

                auto rotate_x = vsg::rotate(rotation_deg.x, vsg::vec3(1.0f, 0.0f, 0.0f));
                auto rotate_y = vsg::rotate(rotation_deg.y, vsg::vec3(0.0f, 1.0f, 0.0f));
                auto rotate_z = vsg::rotate(rotation_deg.z, vsg::vec3(0.0f, 0.0f, 1.0f));
                auto translate = vsg::translate(transform.translation);

                matrix->matrix = translate * rotate_z * rotate_y * rotate_x;
                matrix->addChild(pagedLOD);

                vsg::dvec3 position = vsg::dvec3(matrix->matrix[3][0], matrix->matrix[3][1], matrix->matrix[3][2]);
                world_culling->add(position, matrix);
            }
        }
    }

    route.object_ref.clear();
    route.route_map.clear();

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

    LOG_INFO("Send request for stations data");
    tcp_client->sendRequest(STYPE_REQUEST_STATIONS_DATA);
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
    LOG_INFO("Get route directory name: %s", settings.route_dir_name.c_str());

    loadRoute();

    GUIparams->latitude = route_info.latitude;
    GUIparams->longitude = route_info.longitude;

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

    is_signals = traffic_lights_handler->load(sig_data, settings.route_dir_full_path,
                                              world_culling, options);

    connect(tcp_client.get(), &TcpClient::updateSignal,
            traffic_lights_handler.get(), &TrafficLightsHandler::slotUpdateSignal);

    LOG_INFO("Send request for vehicles info");
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_INFO);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::slotGetStationsData(QByteArray &stations_data)
{
    LOG_INFO("Got stations data from server (%lld bytes)",
             static_cast<long long>(stations_data.size()));

    if (is_stations)
    {
        LOG_WARN("Get stations data again");
        return;
    }
    is_stations = true;

    if (!stations_handler->load(stations_data, options))
    {
        LOG_WARN("Fail to load stations data");
        return;
    }

    stations_handler->setVisible(GUIparams->is_show_HUD && GUIparams->hud_show_stations);

    auto stations_node = stations_handler->getRootNode();

    // Компилируем подграф до подключения его в сцену
    if (!viewer->compileManager)
    {
        LOG_ERROR("No compile manager in viewer");
        return;
    }

    auto compile_result = viewer->compileManager->compile(stations_node);
    if (!compile_result)
    {
        LOG_ERROR("Fail to compile stations scene graph (VkResult %d)",
                  compile_result.result);
        return;
    }

    // Подключаем подграф в сцену и обновляем viewer в фазе update
    viewer->addUpdateOperation(
        Merge::create(viewer, root, stations_node, compile_result),
        vsg::UpdateOperations::ONE_TIME);
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

    connect(tcp_client.get(), &TcpClient::setTrainInfo,
            vehicles_handler.get(), &VehiclesHandler::slotGetTrainsData);

    // Position and state data use DirectConnection — the SPSC ring buffer
    // and atomic flags make these safe without locks
    connect(tcp_client.get(), &TcpClient::setVehiclesPositions,
            vehicles_handler.get(), &VehiclesHandler::slotGetVehiclesPosData, Qt::DirectConnection);

    connect(tcp_client.get(), &TcpClient::setVehiclesData,
            vehicles_handler.get(), &VehiclesHandler::slotGetVehiclesStateData, Qt::DirectConnection);

    connect(tcp_client.get(), &TcpClient::setVehicleControlled,
            vehicles_handler.get(), &VehiclesHandler::slotGetVehicleControlled);

    connect(tcp_client.get(), &TcpClient::setTrainProfile,
            vehicles_handler.get(), &VehiclesHandler::slotGetTrainProfileData, Qt::DirectConnection);

    connect(vehicles_handler.get(), &VehiclesHandler::updated,
            this, &RouteViewer::slotUpdated);

    root->addChild(vehicles_handler->getExterior());

    LOG_INFO("Send request for continuous vehicles update");
    tcp_client->sendRequest(STYPE_REQUEST_TRAINS_UPDATE);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE, static_cast<double>(settings.vehicles_pos_update_interval) * 0.001);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_STATE_UPDATE, static_cast<double>(settings.vehicles_state_update_interval) * 0.001);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLE_CONTROLLED_UPDATE, static_cast<double>(settings.vehicle_controled_update_interval) * 0.001);
    tcp_client->sendTrainProfileRequest(static_cast<double>(settings.train_profile_update_interval) * 0.001,
                                        settings.train_profile_backward,
                                        settings.train_profile_forward);
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
