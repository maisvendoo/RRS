#include    "converter.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProfConverter::ProfConverter()
    : routeDir("")
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProfConverter::~ProfConverter()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int ProfConverter::run(int argc, char *argv[])
{
    switch (parseCommandLine(argc, argv))
    {
    case RESULT_OK:

        break;

    case RESULT_HELP:

        break;

    case RESULT_VERSION:

        break;

    case RESULT_ERROR:

        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool parse_arg(const std::string &arg, cmd_param_t &cmd_param)
{
    if (arg.empty())
        return false;

    char delimiter = '=';
    std::string tmp = arg + delimiter;
    std::vector<std::string> tokens;

    size_t pos = 0;

    while ( (pos = tmp.find(delimiter)) != std::string::npos )
    {
        std::string token = tmp.substr(0, pos);
        tmp.erase(0, pos + 1);
        tokens.push_back(token);
    }

    switch (tokens.size())
    {
    case 1:

        cmd_param.key = tokens[0];
        break;

    case 2:

        cmd_param.key = tokens[0];
        cmd_param.value = tokens[1];
        break;

    default:

        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CmdLineParseResult ProfConverter::parseCommandLine(int argc, char *argv[])
{
    std::vector<std::string> cmd_line;

    for (int i = 0; i < argc; ++i)
        cmd_line.push_back(argv[i]);

    for (auto it = cmd_line.begin() + 1; it != cmd_line.end(); ++it)
    {
        cmd_param_t param;

        if (parse_arg(*it, param))
        {
            if (param.key == "--route")
                routeDir = param.value;

            if (param.key == "--help")
                return RESULT_HELP;

            if (param.key == "--version")
                return RESULT_VERSION;
        }
    }

    if (routeDir.empty())
        return RESULT_ERROR;

    return RESULT_OK;
}
