#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "settings.h"
#include "TrafficLightsHandler.h"

#include <vsg/app/CommandGraph.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/app/Window.h>
#include <vsg/app/WindowTraits.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/lighting/ShadowSettings.h>
#include <vsg/maths/vec4.h>

#include <QObject>

#include <memory>
#include <string>
#include <vsg/nodes/RegionOfInterest.h>

class QString;
class QByteArray;
class SoundManager;
class TcpClient;
class TrainExteriorHandler;

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

    settings_t settings;

    std::unique_ptr<SoundManager> sound_manager;
    std::unique_ptr<TrainExteriorHandler> train_ext_handler;
    std::unique_ptr<TrafficLightsHandler> traffic_lights_handler = std::make_unique<TrafficLightsHandler>();

    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::WindowTraits> windowTraits;
    vsg::ref_ptr<vsg::Window> window;
    vsg::ref_ptr<vsg::LookAt> lookAt;
    vsg::ref_ptr<vsg::Camera> camera;
    vsg::ref_ptr<vsg::Group> root;
    vsg::ref_ptr<vsg::DirectionalLight> sun;
    vsg::ref_ptr<vsg::RegionOfInterest> shadow_region;
    vsg::ref_ptr<vsg::View> view;
    vsg::ref_ptr<vsg::CommandGraph> commandGraph;
    vsg::ref_ptr<vsg::Viewer> viewer;

    TcpClient* tcp_client;
    vsg::ref_ptr<vsg::ShadowSettings> shadowSettings;
};

#endif // ROUTE_VIEWER_H
