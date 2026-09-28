#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <optional>
#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct cmd_line_t
{
    std::optional<std::string> input_route_path;

    std::optional<double> shift_x;
    std::optional<double> shift_y;
    std::optional<double> shift_z;

    std::optional<bool> transform_map;
    std::optional<bool> transform_topology;
};

#endif // COMMAND_LINE_H
