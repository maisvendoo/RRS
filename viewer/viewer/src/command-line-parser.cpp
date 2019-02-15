//------------------------------------------------------------------------------
//
//      Parsing of command line option
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Parsing of command line option
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#include    "command-line-parser.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CommandLineParser::CommandLineParser(int argc, char *argv[])
{
    osg::ArgumentParser args(&argc, argv);

    std::string value = "";

    cmd_line.route_dir.is_present = args.read("--route", cmd_line.route_dir.value);
    cmd_line.train_config.is_present = args.read("--train", cmd_line.train_config.value);
    cmd_line.host_addr.is_present = args.read("--host-addr", cmd_line.host_addr.value);

    if (args.read("--port", value))
        cmd_line.port.is_present = getValue(value, cmd_line.port.value);

    if (args.read("--width", value))
        cmd_line.width.is_present = getValue(value, cmd_line.width.value);

    if (args.read("--height", value))
        cmd_line.height.is_present = getValue(value, cmd_line.height.value);

    cmd_line.fullscreen.is_present = cmd_line.fullscreen.value = args.read("--fullscreen", value);
    cmd_line.localmode.is_present = cmd_line.localmode.value = args.read("--localmode", value);

    cmd_line.notify_level.is_present = args.read("--notify-level", cmd_line.notify_level.value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CommandLineParser::~CommandLineParser()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
cmd_line_t CommandLineParser::getCommadLine() const
{
    return cmd_line;
}
