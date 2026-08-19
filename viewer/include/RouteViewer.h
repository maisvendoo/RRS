#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "settings.h"
#include "graphics-settings.h"

#include <vsg/core/ref_ptr.h>
#include <memory>

class  AnimatedDatabasePager;
class  CfgReader;
class  FileSystem;
class  NewSkybox;
struct GUIParams;
class  QByteArray;
class  ScreenshotWriter;
class  SoundManager;
class  Sun;
class  TcpClient;
class  TrafficLightsHandler;
class  UpdateViewerHandler;
class  VehiclesHandler;
class  WorldCulling;

namespace vsg
{

class AmbientLight;
class Camera;
class CommandGraph;
class Group;
class LookAt;
class Options;
class RegionOfInterest;
class ShadowSettings;
class View;
class Viewer;
class Window;
class WindowTraits;
class PhysicalDevice;

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RouteViewer final : public QObject
{
    Q_OBJECT

public:
    explicit RouteViewer(QObject* parent = nullptr);
    ~RouteViewer();

    void initialize(int argc, char* argv[]);

    int run();

    /// Текущий графический пресет (ТЗ "Графика")
    gfx::Preset getGraphicsPreset() const;

    /// Выбор пресета (в т.ч. из GUI): применяет на лету всё, что
    /// применимо без пересоздания окна; остальное помечается флагом
    /// graphics_needs_restart и сохраняется в settings.xml
    void setGraphicsPreset(gfx::Preset preset, bool save_to_config);

    /// Требуется ли перезапуск для полного применения пресета
    bool isGraphicsRestartRequired() const;

private:
    void loadSettings();
    void loadNetworkSettings(CfgReader& cfg, const QString& section);
    void loadLoggerSettings(CfgReader& cfg, const QString& section);
    void loadModelsSettings(CfgReader& cfg, const QString& section);
    void loadWindowSettings(CfgReader& cfg, const QString& section);
    void loadLightSettings(CfgReader& cfg, const QString& section);
    void loadCameraSettings(CfgReader& cfg, const QString& section);
    void loadFreeCameraSettings(CfgReader& cfg, const QString& section);
    void loadCabineCameraSettings(CfgReader& cfg, const QString& section);
    void loadExternalCameraSettings(CfgReader& cfg, const QString& section);
    void loadFollowCameraSettings(CfgReader& cfg, const QString& section);
    void loadGraphicsSettings(CfgReader& cfg, const QString& section);

    void configureLogLevel() const;

    void overrideSettingsByCommandLine(int argc, char* argv[]);

    void initVsgOptions();
    void initWindowTraits();
    void initWindow(bool try_screenNum_exception = true);
    void initCamera();
    void initScenegraph();

    /// Возможности GPU из выбранного физустройства, автоопределение
    /// пресета и применение QualityParams к настройкам рендера (ТЗ)
    void initGraphicsSettings(const vsg::PhysicalDevice* physDev);

    /// Перенос QualityParams в settings_t (сэмплы, тени, дистанция, LOD)
    void applyGraphicsQualityParams();

    /// Сохранение выбранного пресета в settings.xml (ключ GraphicsPreset)
    void saveGraphicsPresetToConfig(const std::string& preset_name);

    /// Обновление дальности видимости на лету (дальняя плоскость камеры)
    void applyViewDistance(double distance_m);

    /// Погода от симулятора (ТЗ "Видимость и погода"): дальняя плоскость
    /// камеры = min(настройка, видимость), туман - в градиент неба
    void applyWeather();

    /// Динамическое качество: время кадра в ядро настроек (ТЗ)
    void adaptGraphicsQuality(double frame_ms);

    void initLights();
    void configureShaders();

    void initView();
    void initCommandGraph();
    void initViewer();

    void initTcpClient();

    bool loadRoute();

private slots:
    void slotRecvLogMessage(QString msg);

    void slotConnectedToSimulator();

    void slotGetRouteInfoData(QByteArray &data);

    void slotGetSignalsData(QByteArray &sig_data);

    void slotGetVehicleInfoData(QByteArray &data);

    void slotUpdated();

private:
    bool  is_ready = false;
    bool  is_connection_abandoned = false;
    bool  is_route = false;
    bool  is_signals = false;
    bool  is_vehicles = false;

    settings_t settings;

    vsg::ref_ptr<GUIParams>            GUIparams;
    vsg::ref_ptr<UpdateViewerHandler>  upd_viewer_handler;

    std::unique_ptr<TcpClient>             tcp_client;
    std::unique_ptr<SoundManager>          sound_manager;
    std::unique_ptr<ScreenshotWriter>      screenshot_writer;
    std::unique_ptr<TrafficLightsHandler>  traffic_lights_handler;
    std::unique_ptr<VehiclesHandler>       vehicles_handler;
    std::unique_ptr<NewSkybox>             skybox;

    vsg::ref_ptr<vsg::Options>       options;
    vsg::ref_ptr<vsg::WindowTraits>  windowTraits;
    vsg::ref_ptr<vsg::Window>        window;
    vsg::ref_ptr<vsg::LookAt>        lookAt;
    vsg::ref_ptr<vsg::Camera>        camera;
    vsg::ref_ptr<vsg::View>          view;
    vsg::ref_ptr<vsg::CommandGraph>  commandGraph;
    vsg::ref_ptr<vsg::Viewer>        viewer;

    vsg::ref_ptr<vsg::Group>             root;
    vsg::ref_ptr<vsg::ShadowSettings>    shadowSettings;
    vsg::ref_ptr<vsg::RegionOfInterest>  shadow_region;
    vsg::ref_ptr<Sun>                    sun;
    vsg::ref_ptr<WorldCulling>           world_culling;
    vsg::ref_ptr<AnimatedDatabasePager>  database_pager;

    //--------- Настройки графики (ТЗ "Графика") ---------
    gfx::GraphicsSettings graphics_settings;    ///< Ядро пресетов качества

    bool graphics_needs_restart = false;        ///< Перезапуск нужен для полного применения

    int gpu_max_samples = 1;                    ///< Максимум MSAA-сэмплов устройства
    int gpu_max_texture_size = 4096;            ///< Предел размера текстур/карт теней

    /// Пользовательские настройки из settings.xml (снимок до применения
    /// пресета): база для масштабирования и возврат при выборе Custom
    settings_t user_settings;

    /// Параметры, реально baked в окно/пайплайн при старте:
    /// с ними сравниваем новые значения пресета для needs_restart
    int startup_samples = 1;
    bool startup_shadow = false;
    int startup_shadow_cascade = 1;
    int startup_shadow_resolution = 1;

    void checkPhysicalDeviceProperties();
};

#endif // ROUTE_VIEWER_H
