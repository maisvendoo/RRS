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
#include <vsg/lighting/SoftShadows.h>
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
#include <QDomDocument>
#include <QFile>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>
#include <thread>

#include <AltSoundLocker.h>

#include <iostream>

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
                    // Размер 64-битный — формат тоже должен быть 64-битным
                    LOG_INFO("Device's memory[%u] size = %llu MB", heap_idx,
                             static_cast<unsigned long long>(memory_size / 1024 / 1024));
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

    // Снимок пользовательских настроек до того, как их
    // масштабирует графический пресет (для Custom и пересчётов)
    user_settings = settings;

    configureLogLevel();

    LOG_INFO("Override settings from command line");
    overrideSettingsByCommandLine(argc, argv);

    tcp_client = std::make_unique<TcpClient>(this);
    LOG_INFO("Created TcpClient");

    sound_manager = std::make_unique<SoundManager>();
    LOG_INFO("Created SoundManager");

    screenshot_writer = std::make_unique<ScreenshotWriter>("screenshot.jpg");

    traffic_lights_handler = std::make_unique<TrafficLightsHandler>();
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
    auto prev_frame_time = clock::now();

    while (viewer->advanceToNextFrame())
    {
        try
        {
            // Динамическое качество: время кадра в ядро настроек (ТЗ "Графика")
            const auto frame_now = clock::now();
            const double frame_ms = duration<double, std::milli>(frame_now - prev_frame_time).count();
            prev_frame_time = frame_now;
            adaptGraphicsQuality(frame_ms);

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

            // Погода: видимость -> дальняя плоскость, туман -> небо
            applyWeather();

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
        loadWindowSettings(cfg, section);
        loadGraphicsSettings(cfg, section);
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

    // GUI применяет пресет графики через владельца (ТЗ "Графика")
    GUIparams->route_viewer = this;
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
// Флаг количества сэмплов MSAA (используется и при старте, и при
// применении пресета графики к windowTraits)
//------------------------------------------------------------------------------
static VkSampleCountFlags samples_bit_flag(int s)
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RouteViewer::initWindowTraits()
{
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

        // Защита от пустого списка: physDevs.size() - 1 выше дало бы
        // underflow и обращение к physDevs[0] на пустом векторе (UB)
        if (physDevs.empty())
        {
            LOG_FATAL("No Vulkan physical devices found");
            exit(1);
        }

        // Защита от дурака (сравнение без знакового underflow)
        if (settings.physical_device < 0 ||
            static_cast<std::size_t>(settings.physical_device) >= physDevs.size())
        {
            settings.physical_device = 0;
        }

        auto physDev = physDevs[settings.physical_device];

        auto props = physDev->getProperties();
        GUIparams->physicalDeviceName = QString(props.deviceName);

        // Ядро графических настроек: возможности GPU из выбранного
        // устройства, автоопределение пресета и его применение (ТЗ "Графика")
        initGraphicsSettings(physDev.get());

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
// Возможности GPU из свойств выбранного физустройства, автоопределение
// пресета (первый запуск) и применение QualityParams (ТЗ "Графика")
//------------------------------------------------------------------------------
void RouteViewer::initGraphicsSettings(const vsg::PhysicalDevice* physDev)
{
    gfx::GpuCapabilities caps;

    if (physDev)
    {
        const VkPhysicalDeviceProperties& props = physDev->getProperties();

        caps.max_texture_size = static_cast<int>(props.limits.maxImageDimension2D);
        gpu_max_texture_size = caps.max_texture_size;

        // MSAA: пересечение поддерживаемых сэмплов цвета и глубины
        const VkSampleCountFlags sample_counts =
            props.limits.framebufferColorSampleCounts &
            props.limits.framebufferDepthSampleCounts;

        gpu_max_samples = 1;
        if (sample_counts & VK_SAMPLE_COUNT_8_BIT)
        {
            gpu_max_samples = 8;
        }
        else if (sample_counts & VK_SAMPLE_COUNT_4_BIT)
        {
            gpu_max_samples = 4;
        }
        else if (sample_counts & VK_SAMPLE_COUNT_2_BIT)
        {
            gpu_max_samples = 2;
        }

        // VRAM: суммируем кучи с DEVICE_LOCAL
        std::uint64_t vram_bytes = 0;
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(*physDev, &memory_properties);
        for (std::uint32_t heap_idx = 0; heap_idx < memory_properties.memoryHeapCount; ++heap_idx)
        {
            if (memory_properties.memoryHeaps[heap_idx].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                vram_bytes += memory_properties.memoryHeaps[heap_idx].size;
            }
        }
        caps.vram_mb = static_cast<std::size_t>(vram_bytes / (1024ull * 1024ull));

        caps.supports_compute = (physDev->getQueueFamily(VK_QUEUE_COMPUTE_BIT) >= 0);

        // Инстансинг — базовая возможность Vulkan
        caps.supports_instancing = true;

        // Тени у нас рендерятся с depth clamp
        caps.supports_shadow_maps = (physDev->getFeatures().depthClamp == VK_TRUE);

        // float-формат кадра доступен на всех не программных устройствах
        caps.supports_hdr = (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU);

        caps.supports_msaa = (gpu_max_samples > 1);
        caps.supports_geometry_shader = (physDev->getFeatures().geometryShader == VK_TRUE);

        // Апскейлинг FSR-типа выполняется вычислительными шейдерами:
        // доступен на любом устройстве с compute-очередью (Vulkan 1.1+)
        caps.supports_upscaling = caps.supports_compute &&
                                  (props.apiVersion >= VK_API_VERSION_1_1);

        LOG_INFO("GPU capabilities: VRAM %llu MB, max texture %d, MSAA x%d",
                 static_cast<unsigned long long>(caps.vram_mb),
                 caps.max_texture_size,
                 gpu_max_samples);
    }
    else
    {
        LOG_WARN("No physical device for GPU capabilities detection, use defaults");
    }

    // Выбор пресета: Auto — автоопределение (первый запуск),
    // иначе значение из settings.xml
    if (settings.graphics_preset == "Auto")
    {
        const gfx::Preset detected = graphics_settings.autoDetect(caps);
        LOG_INFO("Auto-detected graphics preset: %s",
                 gfx::presetToString(detected).c_str());

        // Запоминаем выбранный пресет в настройки
        settings.graphics_preset = gfx::presetToString(detected);
        saveGraphicsPresetToConfig(settings.graphics_preset);
    }
    else
    {
        graphics_settings.setPreset(gfx::presetFromString(settings.graphics_preset));
        LOG_INFO("Graphics preset from settings: %s", settings.graphics_preset.c_str());
    }

    // Применяем параметры качества пресета к настройкам рендера
    applyGraphicsQualityParams();

    // Сглаживание по пресету — в windowTraits до создания устройства
    windowTraits->samples = samples_bit_flag(settings.samples);

    // Целевое время кадра для динамического качества: учитываем vsync
    // и лимит FPS, иначе адаптация будет бороться с несуществующей целью
    if (settings.vsync || settings.max_fps <= 0)
    {
        // При vsync без лимита ориентируемся на 60 Гц
        graphics_settings.setTargetFrameMs(settings.vsync ? 16.6 : 0.0);
    }
    else
    {
        graphics_settings.setTargetFrameMs(1000.0 / settings.max_fps);
    }

    // Параметры, "запечённые" в окно/пайплайн при старте —
    // с ними сравниваем новые значения при смене пресета на лету
    startup_samples = settings.samples;
    startup_shadow = settings.shadow;
    startup_shadow_cascade = settings.shadow_cascade;
    startup_shadow_resolution = settings.shadow_resolution;

    graphics_needs_restart = false;

    // Настройки для GUI
    GUIparams->graphics_preset_index = static_cast<int>(graphics_settings.getPreset());
}

//------------------------------------------------------------------------------
// Перенос QualityParams в settings_t: сглаживание, тени, дистанция,
// агрессивность LOD. Custom — параметры берутся из settings.xml как есть
//------------------------------------------------------------------------------
void RouteViewer::applyGraphicsQualityParams()
{
    if (graphics_settings.getPreset() == gfx::Preset::Custom)
    {
        // Custom: возвращаем ручные значения пользователя из settings.xml
        settings.samples = user_settings.samples;
        settings.shadow = user_settings.shadow;
        settings.shadow_cascade = user_settings.shadow_cascade;
        settings.shadow_resolution = user_settings.shadow_resolution;
        settings.view_distance = user_settings.view_distance;
        settings.cullingScreenHeightRatio = user_settings.cullingScreenHeightRatio;
        settings.targetPagedLODs = user_settings.targetPagedLODs;
        return;
    }

    const gfx::QualityParams& params = graphics_settings.getParams();

    // Сглаживание: по пресету, но не выше возможностей GPU
    settings.samples = std::min(params.msaa_samples, gpu_max_samples);

    // Тени: включение, каскады и разрешение по пресету
    // (разрешение дополнительно ограничено пределом текстур GPU)
    settings.shadow = params.shadows;
    settings.shadow_cascade = params.shadow_cascades;
    settings.shadow_resolution = std::min(params.shadow_resolution, gpu_max_texture_size);

    // Дальность видимости: масштабируем пользовательское значение
    // от базы (текущий дефолт настроек 2000 м), но не уменьшаем его
    // ниже дистанции самого пресета (ТЗ: Legacy 1500 / Low 3000 /
    // High 5000 / Ultra 8000 м)
    const double base_view_distance = 2000.0;
    const double scale = params.draw_distance_m / base_view_distance;
    settings.view_distance = std::max(user_settings.view_distance * scale,
                                      params.draw_distance_m);

    // Агрессивность LOD-куллинга и лимит подгружаемых PagedLOD:
    // Legacy — максимально агрессивный LOD, Ultra — детальный
    switch (graphics_settings.getPreset())
    {
    case gfx::Preset::Legacy:
        settings.cullingScreenHeightRatio = 0.02;
        settings.targetPagedLODs = 16000;
        break;

    case gfx::Preset::Low:
        settings.cullingScreenHeightRatio = 0.01;
        settings.targetPagedLODs = 32000;
        break;

    case gfx::Preset::High:
        settings.cullingScreenHeightRatio = 0.005;
        settings.targetPagedLODs = 64000;
        break;

    case gfx::Preset::Ultra:
        settings.cullingScreenHeightRatio = 0.002;
        settings.targetPagedLODs = 96000;
        break;

    case gfx::Preset::Custom:
        break;
    }

    settings.cullingScreenHeightRatio = std::clamp(settings.cullingScreenHeightRatio, 0.0, 1.0);
}

//------------------------------------------------------------------------------
// Сохранение пресета графики в settings.xml (ключ GraphicsPreset в Viewer)
//------------------------------------------------------------------------------
void RouteViewer::saveGraphicsPresetToConfig(const std::string& preset_name)
{
    const FileSystem& fs = FileSystem::getInstance();
    const QString cfg_path = QString::fromStdString(
        fs.getConfigDir() + fs.separator() + "settings.xml");

    QFile file(cfg_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LOG_WARN("Fail to open settings.xml for writing graphics preset");
        return;
    }

    QDomDocument doc;
    const bool is_parsed = doc.setContent(&file);
    file.close();

    if (!is_parsed || doc.documentElement().tagName() != "Config")
    {
        LOG_WARN("Fail to parse settings.xml for writing graphics preset");
        return;
    }

    QDomElement root = doc.documentElement();

    // Секция Viewer (создаём при отсутствии)
    QDomElement section = root.firstChildElement("Viewer");
    if (section.isNull())
    {
        section = doc.createElement("Viewer");
        root.appendChild(section);
    }

    // Поле GraphicsPreset (создаём при отсутствии)
    QDomElement field = section.firstChildElement("GraphicsPreset");
    if (field.isNull())
    {
        field = doc.createElement("GraphicsPreset");
        section.appendChild(field);
    }

    // Значение — текст элемента, как читает его CfgReader
    QDomText text_node = field.firstChild().toText();
    if (text_node.isNull())
    {
        text_node = doc.createTextNode(QString::fromStdString(preset_name));
        field.appendChild(text_node);
    }
    else
    {
        text_node.setData(QString::fromStdString(preset_name));
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        LOG_WARN("Fail to write graphics preset to settings.xml");
        return;
    }

    file.write(doc.toString().toUtf8());
    file.close();
}

//------------------------------------------------------------------------------
// Дальняя плоскость отсечения камеры — применяется на лету
//------------------------------------------------------------------------------
void RouteViewer::applyViewDistance(double distance_m)
{
    settings.view_distance = distance_m;

    if (camera)
    {
        if (auto perspective = camera->projectionMatrix->cast<vsg::Perspective>())
        {
            perspective->farDistance = distance_m;
        }
    }
}

//------------------------------------------------------------------------------
// Погода от симулятора (ТЗ "Видимость и погода"): дальняя плоскость
// камеры не дальше дальности видимости, плотность тумана уходит в
// градиент неба. Базовая дальность из настроек не затирается: погода
// каждый кадр берёт min от текущего значения (в т.ч. динамического)
//------------------------------------------------------------------------------
void RouteViewer::applyWeather()
{
    if (!vehicles_handler)
        return;

    const double visibility = vehicles_handler->getWeatherVisibility();

    if (camera && (visibility > 1.0) && (visibility < settings.view_distance))
    {
        if (auto perspective = camera->projectionMatrix->cast<vsg::Perspective>())
        {
            perspective->farDistance = visibility;
        }
    }
    else if (camera)
    {
        // Погода не ограничивает: возвращаем базовую дальность
        if (auto perspective = camera->projectionMatrix->cast<vsg::Perspective>())
        {
            perspective->farDistance = settings.view_distance;
        }
    }

    if (skybox)
    {
        skybox->set_fog(vehicles_handler->getWeatherFogDensity());
    }
}

//------------------------------------------------------------------------------
// Динамическое качество: время кадра в ядро настроек, изменение
// динамической дальности сразу уходит в камеру
//------------------------------------------------------------------------------
void RouteViewer::adaptGraphicsQuality(double frame_ms)
{
    const double prev_distance = graphics_settings.getParams().draw_distance_m;

    graphics_settings.adaptFrameTime(frame_ms);

    // Дальность изменилась заметнее, чем на метр — обновляем камеру.
    // Текущая дальность вида уже несёт старый динамический множитель,
    // поэтому масштабируем отношением новой дистанции к предыдущей
    // (без накопления масштаба от кадра к кадру)
    const double new_distance = graphics_settings.getParams().draw_distance_m;
    if (prev_distance > 0.0 &&
        std::abs(new_distance - prev_distance) > 1.0)
    {
        applyViewDistance(settings.view_distance * (new_distance / prev_distance));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
gfx::Preset RouteViewer::getGraphicsPreset() const
{
    return graphics_settings.getPreset();
}

//------------------------------------------------------------------------------
// Смена пресета (в т.ч. из GUI): применяем на лету всё, что можно,
// остальное помечаем флагом needs_restart и сохраняем в конфиг
//------------------------------------------------------------------------------
void RouteViewer::setGraphicsPreset(gfx::Preset preset, bool save_to_config)
{
    if (preset == gfx::Preset::Custom)
    {
        // Фиксируем текущие параметры ядра как ручную базу
        graphics_settings.applyPreset(gfx::Preset::Custom);
    }
    else
    {
        graphics_settings.setPreset(preset);
    }

    graphics_settings.resetDynamic();
    applyGraphicsQualityParams();

    // --- Применение на лету ---

    // Дальность видимости: дальняя плоскость камеры
    applyViewDistance(settings.view_distance);

    // Агрессивность LOD: поля DatabasePager'а читаются в рантайме
    if (database_pager)
    {
        database_pager->cullingScreenHeightRatio = settings.cullingScreenHeightRatio;
        database_pager->targetMaxNumPagedLODWithHighResSubgraphs = settings.targetPagedLODs;
    }

    // --- Требующие пересоздания окна/пайплайна: применится после перезапуска ---
    graphics_needs_restart =
        (settings.samples != startup_samples) ||
        (settings.shadow != startup_shadow) ||
        (settings.shadow_cascade != startup_shadow_cascade) ||
        (settings.shadow_resolution != startup_shadow_resolution);

    settings.graphics_preset = gfx::presetToString(preset);
    GUIparams->graphics_preset_index = static_cast<int>(preset);
    GUIparams->graphics_needs_restart = graphics_needs_restart;

    if (save_to_config)
    {
        saveGraphicsPresetToConfig(settings.graphics_preset);
    }

    LOG_INFO("Graphics preset changed: %s%s", settings.graphics_preset.c_str(),
             graphics_needs_restart ? " (restart required for full apply)" : "");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RouteViewer::isGraphicsRestartRequired() const
{
    return graphics_needs_restart;
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
        // Тип теней по пресету (ТЗ "Графика"): High/Ultra — мягкие,
        // Legacy/Low/Custom — жёсткие (существующий путь)
        const gfx::Preset preset = graphics_settings.getPreset();

        if ((preset == gfx::Preset::High) || (preset == gfx::Preset::Ultra))
        {
            shadowSettings = vsg::SoftShadows::create(settings.shadow_cascade);
        }
        else
        {
            shadowSettings = vsg::HardShadows::create(settings.shadow_cascade);
        }

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
    database_pager = AnimatedDatabasePager::create();
    database_pager->targetMaxNumPagedLODWithHighResSubgraphs = settings.targetPagedLODs;
    database_pager->cullingScreenHeightRatio = settings.cullingScreenHeightRatio;
    for (auto& task : viewer->recordAndSubmitTasks)
    {
        task->databasePager = database_pager;
    }

    // Перед компиляцией вьювера применяем некоторые настройки
    auto resourceHints = vsg::ResourceHints::create();
    // Указываем количество потоков чтения 3d-моделей
    uint32_t numThreads = std::max(1u, std::thread::hardware_concurrency() / 2);
    uint32_t numReadThreads = std::min(settings.read_threads, numThreads);
    resourceHints->numDatabasePagerReadThreads = numReadThreads;
    // Указываем разрешение карты теней (ограничиваем пределом текстур GPU,
    // он же клампит разрешение пресета — ТЗ "Графика")
    const auto shadow_map_size = static_cast<uint32_t>(
        std::min(settings.shadow_resolution, gpu_max_texture_size));
    resourceHints->shadowMapSize = {shadow_map_size, shadow_map_size};
    // Указываем допустимое количество источников света
    resourceHints->numLightsRange = {static_cast<uint32_t>(settings.num_lights),
                                     static_cast<uint32_t>(settings.num_lights + 1)};
    /*auto compileResult = */viewer->compile(resourceHints);
    /*if (!compileResult)
    {
        LOG_WARN("Viewer compile returned empty result — some resources may not have been compiled");
    }

    // Задаем лимит выделение видеопамяти (для разработчиков! отладка работы на слабых системах!)
    auto device = window->getDevice();
    auto memPolls = device->deviceMemoryBufferPools.ref_ptr();

    if (memPolls)
    {
        memPolls->allocatedMemoryLimit = std::clamp(settings.allocatedMemoryLimit, 0.0, 1.0);
    }*/

    // Создаём вспомогательные потоки для чтения текстур 3d-моделей
    uint32_t numOpThreads = std::min(settings.operation_threads, numThreads);
    if (numOpThreads)
    {
        options->operationThreads = vsg::OperationThreads::create(numOpThreads, viewer->status);
    }

    GUIparams->viewer = viewer;
    GUIparams->vehicles_handler = vehicles_handler.get();
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

    connect(tcp_client.get(), &TcpClient::connected, this, &RouteViewer::slotConnectedToSimulator);
    connect(tcp_client.get(), &TcpClient::setRouteInfo, this, &RouteViewer::slotGetRouteInfoData);
    connect(tcp_client.get(), &TcpClient::setSignalsData, this, &RouteViewer::slotGetSignalsData);
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

    // Снимок диагностики составов (ТЗ "Промт статистики вагонов", F3/F4)
    connect(tcp_client.get(), &TcpClient::setDiagnosticsData,
            vehicles_handler.get(), &VehiclesHandler::slotGetDiagnosticsData,
            Qt::DirectConnection);

    connect(vehicles_handler.get(), &VehiclesHandler::sigSendVehicleControlCommand,
            tcp_client.get(), &TcpClient::slotSendVehicleControlCommand);

    connect(vehicles_handler.get(), &VehiclesHandler::updated,
            this, &RouteViewer::slotUpdated);

    root->addChild(vehicles_handler->getExterior());

    LOG_INFO("Send request for continuous vehicles update");
    tcp_client->sendRequest(STYPE_REQUEST_TRAINS_UPDATE);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE, static_cast<double>(settings.vehicles_pos_update_interval) * 0.001);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_STATE_UPDATE, static_cast<double>(settings.vehicles_state_update_interval) * 0.001);
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLE_CONTROLLED_UPDATE, static_cast<double>(settings.vehicle_controled_update_interval) * 0.001);

    // Диагностика составов (ТЗ "Промт статистики вагонов"): снимок раз в 0.5 с
    tcp_client->sendRequest(STYPE_REQUEST_DIAGNOSTICS_UPDATE, 0.5);
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
