//-----------------------------------------------------------------------------
//
//
//
//
//
//-----------------------------------------------------------------------------
#ifndef     LAUNCHER_COMMAND_LINE_H
#define     LAUNCHER_COMMAND_LINE_H

#include    "command-line.h"
#include    <QString>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct launcher_command_line_t
{
    option_t<int>       width;
    option_t<int>       height;
    option_t<bool>      fullscreen;
    option_t<QString>   host_address;
    option_t<int>       port;
    option_t<bool>      remote_client;
};

#endif // LAUNCHER_COMMAND_LINE_H
