#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <core/Option.h>

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct cmd_line_t
{
    option_t<std::string> input_route_path;

    option_t<double> shift_x;
    option_t<double> shift_y;
    option_t<double> shift_z;

    option_t<bool> transform_map;
    option_t<bool> transform_topology;
};

#endif // COMMAND_LINE_H
