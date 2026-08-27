#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "settings.h"

#include <vsg/core/ref_ptr.h>
#include <memory>

class  CfgReader;
class  FileSystem;
class  NewSkybox;
struct GUIParams;
class  QByteArray;
class  ScreenshotWriter;
class  SoundManager;
class  StationsHandler;
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

private:
    void loadSettings();
    void loadNetworkSettings(CfgReader& cfg, const QString& section);
    void loadLoggerSettings(CfgReader& cfg, const QString& section);
    void loadModelsSettings(CfgReader& cfg, const QString& section);
    void loadStationsTextSettings(CfgReader& cfg, const QString& section);
    void loadWindowSettings(CfgReader& cfg, const QString& section);
    void loadHUDSettings(CfgReader& cfg, const QString& section);
    void loadLightSettings(CfgReader& cfg, const QString& section);
    void loadCameraSettings(CfgReader& cfg, const QString& section);
    void loadFreeCameraSettings(CfgReader& cfg, const QString& section);
    void loadCabineCameraSettings(CfgReader& cfg, const QString& section);
    void loadExternalCameraSettings(CfgReader& cfg, const QString& section);
    void loadFollowCameraSettings(CfgReader& cfg, const QString& section);

    void configureLogLevel() const;

    void overrideSettingsByCommandLine(int argc, char* argv[]);

    void initVsgOptions();
    void initWindowTraits();
    void initWindow(bool try_screenNum_exception = true);
    void initCamera();
    void initScenegraph();

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

    void slotGetStationsData(QByteArray &stations_data);

    void slotGetVehicleInfoData(QByteArray &data);

    void slotUpdated();

private:
    bool  is_ready = false;
    bool  is_connection_abandoned = false;
    bool  is_route = false;
    bool  is_signals = false;
    bool  is_stations = false;
    bool  is_vehicles = false;

    settings_t settings;

    vsg::ref_ptr<GUIParams>            GUIparams;
    vsg::ref_ptr<UpdateViewerHandler>  upd_viewer_handler;

    std::unique_ptr<TcpClient>             tcp_client;
    std::unique_ptr<SoundManager>          sound_manager;
    std::unique_ptr<ScreenshotWriter>      screenshot_writer;
    std::unique_ptr<StationsHandler>       stations_handler;
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
void checkPhysicalDeviceProperties();
};

#endif // ROUTE_VIEWER_H
