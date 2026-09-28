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
    std::optional<bool> input_only_used_at_map;
    std::optional<bool> input_lights_at_map;
    std::optional<bool> input_compress_textures;
    std::optional<bool> smooth;
    std::optional<int> num_threads;

    std::optional<std::string> output_route_path;

    std::optional<std::string> input_model_path;
    std::optional<std::string> input_texture_path;

    std::optional<std::string> output_model_path;
};

#endif // COMMAND_LINE_H
