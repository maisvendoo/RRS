#include    <Application.h>
#include    <Logger.h>

#include    <filesystem>
#include    <fstream>
#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::parse_args(int argc, char* argv[])
{
    cli::Parser parser(argc, argv);

    configure_parser(parser);

    parse_command_line(parser, cmd_line);

    return check_command_line(cmd_line);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::configure_parser(cli::Parser &parser)
{
    parser.set_optional<std::string>("i", "input-route",
                                     "",
                                     "Input route path");

    parser.set_optional<bool>("m", "map-transform",
                              false,
                              "Transform positions at files /topology/map/*.map");

    parser.set_optional<bool>("t", "topology-transform",
                              false,
                              "Transform positions at files /topology/trajectories/*.traj");

    parser.set_optional<double>("x", "delta-x",
                                0.0,
                                "Transform along/aroung X axis");

    parser.set_optional<double>("y", "delta-y",
                                0.0,
                                "Transform along/aroung Y axis");

    parser.set_optional<double>("z", "delta-z",
                                0.0,
                                "Transform along/aroung Z axis");

    parser.enable_help();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::parse_command_line(cli::Parser &parser, cmd_line_t &cmd_line)
{
    parser.run_and_exit_if_error();
    cmd_line.input_route_path = parser.get<std::string>("i");
    cmd_line.transform_map = parser.get<bool>("m");
    cmd_line.transform_topology = parser.get<bool>("t");
    cmd_line.shift_x = parser.get<double>("x");
    cmd_line.shift_y = parser.get<double>("y");
    cmd_line.shift_z = parser.get<double>("z");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::check_command_line(const cmd_line_t &cmd_line)
{
    if (!cmd_line.input_route_path.isPresent())
    {
        LOG_WARN("ERROR: Missing input route path");
        return false;
    }

    if (!std::filesystem::exists(cmd_line.input_route_path.value))
    {
        LOG_WARN("ERROR: input route path not exists");
        return false;
    }

    if (cmd_line.transform_map.value || cmd_line.transform_topology.value)
    {
        return true;
    }
    else
    {
        LOG_WARN("ERROR: Nothing to do, options --map-transform or --topology-transform are missing");
        return false;
    }
    return true;
}
