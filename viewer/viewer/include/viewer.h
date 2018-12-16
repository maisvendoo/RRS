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

#include    <osgViewer/Viewer>

#include    "settings.h"
#include    "command-line-parser.h"
#include    "qt-events.h"
#include    "network.h"
#include    "route-info.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RouteViewer : public QObject
{
    Q_OBJECT

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

    unsigned int                current_route_id;

    /// Viewer ready flag
    bool                        is_ready;
    /// Viewer settings
    settings_t                  settings;

    /// OSG viewer object
    osgViewer::Viewer           viewer;          

    /// TCP-client
    NetworkClient               client;

    /// Initialization
    bool init(int argc, char *argv[]);   

    /// Load settings from config
    settings_t loadSettings(const std::string &cfg_path) const;

    /// Override settings from command line
    void overrideSettingsByCommandLine(const cmd_line_t &cmd_line,
                                       settings_t settings);    

    /// Init display
    bool initDisplay(osgViewer::Viewer *viewer, const settings_t &settings);

    /// Init motion blur
    bool initMotionBlurEffect(osgViewer::Viewer *viewer, const settings_t &settings);    
};

#endif // VIEWER_H
