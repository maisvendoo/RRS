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

#include    "simulator-info-struct.h"

#include    "settings.h"
#include    "command-line-parser.h"
#include    "client.h"

#include    "keyboard.h"

#include    "train-exterior.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RouteViewer
{
public:

    /// Constructor
    RouteViewer(int argc, char *argv[]);

    /// Destructor
    virtual ~RouteViewer();

    /// Get ready state
    bool isReady() const;

    /// Start client
    int run();

protected:

    /// Viewer ready flag
    bool                        is_ready;

    QSharedMemory   memory_sim_info;
    simulator_info_t info_data;

    KeyboardHandler             *keyboard;

    /// Viewer settings
    settings_t                  settings;

    /// OSG viewer object
    osgViewer::Viewer           viewer;

    /// OSG scene root node
    osg::ref_ptr<osg::Group>    root;

    NetworkClient               client;


    TrainExteriorHandler *train_ext_handler;

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
};

#endif // VIEWER_H
