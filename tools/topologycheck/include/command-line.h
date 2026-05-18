#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <optional>
#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct cmd_line_t
{
    std::optional<std::string> route_path;
    std::optional<double> minimum_curve_radius;
};

#endif // COMMAND_LINE_H
