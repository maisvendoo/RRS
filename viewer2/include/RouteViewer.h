#ifndef ROUTE_VIEWER_H
#define ROUTE_VIEWER_H

#include "SoundManager.h"
#include "TrainExteriorHandler.h"
#include "settings.h"
#include "tcp-client.h"

#include <memory>
#include <qobject.h>
#include <qtcoreexports.h>
#include <qtmetamacros.h>
#include <string>
#include <vsg/app/RecordTraversal.h>
#include <vsg/app/Viewer.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

class RouteViewer : public QObject
{
    Q_OBJECT

public:
    RouteViewer(int argc, char* argv[], QObject* parent = Q_NULLPTR);
    ~RouteViewer() = default;

    bool isReady() const;

    int run();

private:
    bool init(int argc, char* argv[]);

    void loadSettings(const std::string& cfg_path);

    int overrideSettingsByCommandLine(int argc, char* argv[]);

    bool initEngineSettings();

    void initEnvironmentLight(vsg::vec4 color, float power, float psi, float theta);

    bool initDisplay();

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

    vsg::ref_ptr<vsg::Group> root;

    vsg::ref_ptr<vsg::Viewer> viewer;

    TcpClient* tcp_client;
};

#endif // ROUTE_VIEWER_H
