#ifndef     COMMAND_LINE_H
#define     COMMAND_LINE_H

#include    <optional>
#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct command_line_t
{
    std::optional<std::string> model_path;
    bool generate_mipmaps = false;
    std::optional<std::string> skip_textures;
    bool overwrite_gltf = false;
    bool ignore_existed = false;
    bool delete_src = false;
    bool extract = false;
};

#endif // COMMAND_LINE_H
