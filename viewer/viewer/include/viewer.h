//------------------------------------------------------------------------------
//
//      Video client's window manager
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Video client's window manager
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#ifndef     VIEWER_H
#define     VIEWER_H

#include    <QSharedMemory>
#include    <osgViewer/Viewer>

#include    <simulator-info-struct.h>

#include    <settings.h>
#include    <command-line-parser.h>

#include    <keyboard.h>

#include    <train-exterior.h>
#include    <sound-manager.h>

#include    <tcp-client.h>

#include    <traffic-lights-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RouteViewer : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    RouteViewer(int argc, char *argv[], QObject *parent = Q_NULLPTR);

    /// Destructor
    virtual ~RouteViewer();

    /// Get ready state
    bool isReady() const;

    /// Start client
    int run();

protected:

    /// Viewer ready flag
    bool                        is_ready = false;

    QSharedMemory   memory_sim_info;
    simulator_info_t info_data;

    KeyboardHandler             *keyboard = nullptr;

    /// Viewer settings
    settings_t                  settings;

    /// OSG viewer object
    osgViewer::Viewer           viewer;

    /// OSG scene root node
    osg::ref_ptr<osg::Group>    root;

    TrainExteriorHandler *train_ext_handler = Q_NULLPTR;

    /// Sound manager
    SoundManager *sound_manager = Q_NULLPTR;

    /// TCP-client
    TcpClient *tcp_client = new TcpClient;

    /// Process traffic lights (signals) models
    osg::ref_ptr<TrafficLightsHandler> traffic_lights_handler = new TrafficLightsHandler;

    /// Initialization
    bool init(int argc, char *argv[]);   

    /// Load settings from config
    settings_t loadSettings(const std::string &cfg_path) const;

    /// Override settings from command line
    void overrideSettingsByCommandLine(const cmd_line_t &cmd_line,
                                       settings_t &settings);

    /// Override settings from shared memory with simulator info
    void overrideSettingsBySharedMemory(settings_t &settings);

    /// Load route form directory
    bool loadRoute();

    /// Init common graphical engine settings
    bool initEngineSettings(osg::Group *root);

    /// Init display
    bool initDisplay(osgViewer::Viewer *viewer, const settings_t &settings);

    /// Init TCP-client
    void initTCPclient(const settings_t &settings);

protected slots:

    void slotConnectedToSimulator();

    void slotGetSignalsData(QByteArray &sig_data);
};

#endif // VIEWER_H
