#include    <app.h>
#include    <converter.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Application::run(int argc, char *argv[])
{
    if (!init(argc, argv))
    {
        return -1;
    }

    Converter conv;

    return conv.run(cmd_line);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::init(int argc, char *argv[])
{
    cli::Parser parser(argc, argv);

    configure_parser(parser);

    parse_command_line(parser, cmd_line);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::configure_parser(cli::Parser &parser)
{
    parser.set_optional<std::string>("m", "model-path",
                                     "",
                                     "glTF 2.0 model file");

    parser.set_optional<bool>("g", "generate-mipmaps",
                              false,
                              "Generate mipmaps");

    parser.set_optional<std::string>("s", "skip-textures",
                                     "",
                                     "Skip textures by name (<name1>,<name2>,...,<name_N>)");

    parser.set_optional<bool>("o", "overwrite-gltf",
                              false,
                              "Overwrite source GLTF file");

    parser.set_optional<bool>("i", "ignore-existed-ktx2",
                              false,
                              "Not rewrite existed KTX2 texture");

    parser.set_optional<bool>("d", "delete-src-texture",
                              false,
                              "Delete source texture");

    parser.enable_help();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::parse_command_line(cli::Parser &parser,
                                     command_line_t &cmd_line)
{
    parser.run_and_exit_if_error();

    cmd_line.model_path.value = parser.get<std::string>("m");
    cmd_line.model_path.is_present = !cmd_line.model_path.value.empty();

    cmd_line.generate_mipmaps = parser.get<bool>("g");

    cmd_line.skip_textures.value = parser.get<std::string>("s");
    cmd_line.skip_textures.is_present = !cmd_line.skip_textures.value.empty();

    cmd_line.overwrite_gltf = parser.get<bool>("o");

    cmd_line.ignore_existed = parser.get<bool>("i");

    cmd_line.delete_src = parser.get<bool>("d");

    return true;
}
