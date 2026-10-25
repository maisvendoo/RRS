#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <core/Option.h>

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct cmd_line_t
{
    option_t<std::string> route_path;
    option_t<double> minimum_curve_radius;
};

#endif // COMMAND_LINE_H
