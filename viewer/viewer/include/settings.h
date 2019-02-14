//------------------------------------------------------------------------------
//
//      Viewer settings structure (from settings.xml file)
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Viewer settings structure (from settings.xml file)
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#ifndef     SETTINGS_H
#define     SETTINGS_H

#include    <string>

/*!
 * \struct
 * \brief Main viewer settings
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct settings_t
{
    /// Route directory
    std::string     route_dir;
    /// Train config file name
    std::string     train_config;
    /// Server ip-address
    std::string     host_addr;
    /// Server port
    int             port;
    /// Window horizontal position
    int             x;
    /// Window vertical position
    int             y;
    /// Window width
    int             width;
    /// Window height
    int             height;
    /// Fullscreen flag
    bool            fullscreen;
    ///
    bool            vsync;
    /// Work in localmode (reserved)
    bool            localmode;
    /// Vertical view angle
    double          fovy;
    ///
    double          zNear;
    ///
    double          zFar;
    /// Screen number
    unsigned int    screen_number;
    /// Client name for server autorization
    std::string     name;
    /// Set/unset window decorations
    bool            window_decoration;
    /// Set/unset double buffering
    bool            double_buffer;
    /// Set number of anialiasing samples
    bool            samples;
    /// Set client's data request time interval
    int             request_interval;
    /// Set client reconnection interval
    int             reconnect_interval;
    /// Motion blur persistence
    double          persistence;
    /// Cabine driver's eye height
    float           eye_height;
    /// Route motion direction
    int             direction;

    settings_t()
        : route_dir("")
        , train_config("")
        , host_addr("127.0.0.1")
        , port(1992)
        , x(50)
        , y(50)
        , width(1366)
        , height(768)
        , fullscreen(false)
        , vsync(true)
        , localmode(false)
        , fovy(30.0)
        , zNear(1.0)
        , zFar(1000.0)
        , screen_number(0)
        , name("viewer")
        , window_decoration(true)
        , double_buffer(true)
        , samples(4)
        , request_interval(200)
        , reconnect_interval(1000)
        , persistence(0.05)
        , eye_height(3.0)
        , direction(1)
    {

    }
};

#endif // SETTINGS_H
