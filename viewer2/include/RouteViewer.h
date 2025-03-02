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
    void initWindow(bool try_screenNum_exception = true);
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

    settings_t settings = settings_t();

    TcpClient *tcp_client = nullptr;
    SoundManager *sound_manager = nullptr;
    TrafficLightsHandler *traffic_lights_handler = nullptr;
    VehiclesHandler *vehicles_handler = nullptr;

    vsg::ref_ptr<vsg::Options> options = nullptr;
    vsg::ref_ptr<vsg::WindowTraits> windowTraits = nullptr;
    vsg::ref_ptr<vsg::Window> window = nullptr;
    vsg::ref_ptr<vsg::LookAt> lookAt = nullptr;
    vsg::ref_ptr<vsg::Camera> camera = nullptr;
    vsg::ref_ptr<vsg::Group> root = nullptr;
    vsg::ref_ptr<vsg::DirectionalLight> sun = nullptr;
    vsg::ref_ptr<vsg::ShadowSettings> shadowSettings = nullptr;
    vsg::ref_ptr<vsg::RegionOfInterest> shadow_region = nullptr;
    vsg::ref_ptr<vsg::View> view = nullptr;
    vsg::ref_ptr<vsg::CommandGraph> commandGraph = nullptr;
    vsg::ref_ptr<vsg::Viewer> viewer = nullptr;
};

#endif // ROUTE_VIEWER_H
