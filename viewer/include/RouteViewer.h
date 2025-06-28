#pragma once
#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "MyGui.h"
#include "UpdateViewerHandler.h"
#include "settings.h"

#include <vsg/app/CommandGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/ShadowSettings.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/RegionOfInterest.h>

class CfgReader;
class QByteArray;
class SoundManager;
class TcpClient;
class ScreenshotWriter;
class TrafficLightsHandler;
class VehiclesHandler;
class UpdateViewerHandler;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RouteViewer final : public QObject
{
    Q_OBJECT

public:
    RouteViewer(int argc, char* argv[], QObject* parent = Q_NULLPTR);
    ~RouteViewer();

    bool isReady() const;

    int run();

private:
    bool init(int argc, char* argv[]);

    void loadSettings();
    void loadNetworkSettings(CfgReader& cfg, const QString& section);
    void loadLoggerSettings(CfgReader& cfg, const QString& section);
    void loadWindowSettings(CfgReader& cfg, const QString& section);
    void loadLightSettings(CfgReader& cfg, const QString& section);
    void loadCameraSettings(CfgReader& cfg, const QString& section);
    void loadFreeCameraSettings(CfgReader& cfg, const QString& section);
    void loadCabineCameraSettings(CfgReader& cfg, const QString& section);
    void loadExternalCameraSettings(CfgReader& cfg, const QString& section);
    void loadFollowCameraSettings(CfgReader& cfg, const QString& section);

    void configureLogLevel();

    void overrideSettingsByCommandLine(int argc, char* argv[]);

    void initVsgOptions();
    void initWindowTraits();
    void initWindow(bool try_screenNum_exception = true);
    void initCamera();
    void initScenegraph();
    void initLights();
    void initView();
    void initCommandGraph();
    void initViewer();

    void initTCPclient();

    bool loadRoute();

private slots:
    void slotRecvLogMessage(QString msg);

    void slotConnectedToSimulator();

    void slotGetRouteInfoData(QByteArray &data);

    void slotGetSignalsData(QByteArray &sig_data);

    void slotGetVehicleInfoData(QByteArray &data);

    void slotUpdated();

private:
    bool is_ready = false;
    bool is_route = false;
    bool is_signals = false;
    bool is_vehicles = false;

    settings_t settings = settings_t();

    vsg::ref_ptr<GUIParams> GUIparams = nullptr;
    vsg::ref_ptr<UpdateViewerHandler> upd_viewer_handler = nullptr;

    // Replace by smart pointers?
    TcpClient *tcp_client = nullptr;
    SoundManager *sound_manager = nullptr;
    ScreenshotWriter *screenshot_writer = nullptr;
    TrafficLightsHandler *traffic_lights_handler = nullptr;
    VehiclesHandler *vehicles_handler = nullptr;

    vsg::ref_ptr<vsg::Options> options = nullptr;
    vsg::ref_ptr<vsg::WindowTraits> windowTraits = nullptr;
    vsg::ref_ptr<vsg::Window> window = nullptr;
    vsg::ref_ptr<vsg::LookAt> lookAt = nullptr;
    vsg::ref_ptr<vsg::Camera> camera = nullptr;
    vsg::ref_ptr<vsg::View> view = nullptr;
    vsg::ref_ptr<vsg::CommandGraph> commandGraph = nullptr;
    vsg::ref_ptr<vsg::Viewer> viewer = nullptr;

    vsg::ref_ptr<vsg::Group> root = nullptr;
    vsg::ref_ptr<vsg::AmbientLight> ambient = nullptr;
    vsg::ref_ptr<vsg::DirectionalLight> sun = nullptr;
    vsg::ref_ptr<vsg::ShadowSettings> shadowSettings = nullptr;
    vsg::ref_ptr<vsg::RegionOfInterest> shadow_region = nullptr;
};

#endif // ROUTE_VIEWER_H
