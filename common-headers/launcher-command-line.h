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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct launcher_command_line_t
{
    option_t<int>   width;
    option_t<int>   height;
    option_t<bool>  fullscreen;
};

#endif // LAUNCHER_COMMAND_LINE_H
