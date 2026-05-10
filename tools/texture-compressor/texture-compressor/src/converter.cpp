#include    <converter.h>
#include    <compressor.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Converter::run(const command_line_t &cmd_line)
{
    if (cmd_line.extract)
    {
        return 0;
    }

    return compress_model_textures(cmd_line);
}
