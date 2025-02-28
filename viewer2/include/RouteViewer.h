#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "settings.h"

#include <vsg/app/CommandGraph.h>
#include <vsg/app/View.h>
#include <vsg/app/Viewer.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/lighting/ShadowSettings.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/RegionOfInterest.h>

#include <string>

class QByteArray;
class SoundManager;
class TcpClient;
class TrafficLightsHandler;
class VehiclesHandler;
//class InputHandler;

namespace vsg
{
    class Group;
    class Viewer;
}

class RouteViewer : public QObject
{
    Q_OBJECT

public:
    RouteViewer(int argc, char* argv[], QObject* parent = Q_NULLPTR);
    ~RouteViewer();

    bool isReady() const;

    int run();

private:
    bool init(int argc, char* argv[]);

    void loadSettings(const std::string& cfg_path);

    int overrideSettingsByCommandLine(int argc, char* argv[]);

    void initVsgOptions();
    void initWindowTraits();
    void initWindow();
    void initCamera();
    void initScenegraph();
    void initLights();
    void initView();
    void initCommandGraph();
    void initViewer();

    // bool initEngineSettings();

    // void initEnvironmentLight(vsg::vec4 color, float power, float psi, float theta);

    // bool initDisplay();


    void initTCPclient();

    bool loadRoute();

private slots:
    void slotRecvLogMessage(QString msg);

    void slotConnectedToSimulator();

    void slotGetRouteInfoData(QByteArray &data);

    void slotGetSignalsData(QByteArray &sig_data);

    void slotGetVehicleInfoData(QByteArray &data);

    void slotUpdateKeyboard();

    void slotUpdateControlledVehicle();

private:
    bool is_ready = false;
    bool is_route = false;
    bool is_signals = false;
    bool is_vehicles = false;

    settings_t settings;

    TcpClient* tcp_client;
    SoundManager *sound_manager;
    TrafficLightsHandler *traffic_lights_handler;
    VehiclesHandler *vehicles_handler;

    //vsg::ref_ptr<InputHandler> vehicles_update_handler;

    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::WindowTraits> windowTraits;
    vsg::ref_ptr<vsg::Window> window;
    vsg::ref_ptr<vsg::LookAt> lookAt;
    vsg::ref_ptr<vsg::Camera> camera;
    vsg::ref_ptr<vsg::Group> root;
    vsg::ref_ptr<vsg::DirectionalLight> sun;
    vsg::ref_ptr<vsg::ShadowSettings> shadowSettings;
    vsg::ref_ptr<vsg::RegionOfInterest> shadow_region;
    vsg::ref_ptr<vsg::View> view;
    vsg::ref_ptr<vsg::CommandGraph> commandGraph;
    vsg::ref_ptr<vsg::Viewer> viewer;
};

#endif // ROUTE_VIEWER_H
