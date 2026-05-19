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
    std::optional<std::string> input_route_path;
    std::optional<std::string> output_route_path;
};

#endif // COMMAND_LINE_H
