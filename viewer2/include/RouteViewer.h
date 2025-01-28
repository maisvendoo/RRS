#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "settings.h"

#include <vsg/app/Window.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/maths/vec4.h>

#include <QObject>

#include <memory>
#include <string>

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

    bool initEngineSettings();

    void initEnvironmentLight(vsg::vec4 color, float power, float psi, float theta);

    bool initDisplay();

    void initCamera();

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
    bool is_ready;
    bool is_route;

    settings_t settings;

    std::unique_ptr<SoundManager> sound_manager;
    std::unique_ptr<TrainExteriorHandler> train_ext_handler;

    vsg::ref_ptr<vsg::Options> options;
    vsg::ref_ptr<vsg::Group> root;
    vsg::ref_ptr<vsg::Window> window;
    vsg::ref_ptr<vsg::Viewer> viewer;
    vsg::ref_ptr<vsg::Camera> camera;

    TcpClient* tcp_client;
};

#endif // ROUTE_VIEWER_H
