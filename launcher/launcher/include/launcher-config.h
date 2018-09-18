//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     LAUNCHER_CONFIG_H
#define     LAUNCHER_CONFIG_H

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct launcher_config_t
{
    int     width;
    int     height;
    bool    fullscreen;
    QString host_address;
    int     port;

    launcher_config_t()
        : width(1368)
        , height(768)
        , fullscreen(false)
        , host_address("127.0.0.1")
        , port(1992)
    {

    }
};

#endif // LAUNCHERCONFIG_H
