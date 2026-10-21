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
    option_t<std::string> input_route_path;
    option_t<std::string> output_route_path;
};

#endif // COMMAND_LINE_H
