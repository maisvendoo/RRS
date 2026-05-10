#include    <converter.h>
#include    <compressor.h>
#include    <extractor.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Converter::run(const command_line_t &cmd_line)
{
    if (cmd_line.extract)
    {
        return extract_models_textures(cmd_line);
    }

    return compress_model_textures(cmd_line);
}
