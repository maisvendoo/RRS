#ifndef     CONVERTER_H
#define     CONVERTER_H

#include    <command-line.h>
#include    <filesystem>

namespace fs = std::filesystem;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Converter
{
public:

    Converter() = default;

    ~Converter() = default;

    int run(const command_line_t &cmd_line);

private:

    bool compress_to_ktx2(const fs::path& src_img, const fs::path& out_ktx);
};

#endif
