//------------------------------------------------------------------------------
//
//      Launcher configuration
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Launcher configuration
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#ifndef     LAUNCHER_CONFIG_H
#define     LAUNCHER_CONFIG_H

#include    <QString>

/*!
 * \struct
 * \brief Configuration parameters
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct launcher_config_t
{
    /// Window width
    int     width;
    /// Window height
    int     height;
    /// Fullscreen flag
    bool    fullscreen;
    /// Server IP-address
    QString host_address;
    /// Server port for connection
    int     port;
    /// Start as remote client
    bool    remote_client;

    launcher_config_t()
        : width(1368)
        , height(768)
        , fullscreen(false)
        , host_address("127.0.0.1")
        , port(1992)
        , remote_client(false)
    {

    }
};

#endif // LAUNCHERCONFIG_H
