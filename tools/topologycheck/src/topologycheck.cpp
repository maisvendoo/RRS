#include    "topologycheck.h"
#include    "command-line.h"
#include    "Logger.h"

#include    <QString>
#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int TopologyCheck::run(int argc, char *argv[])
{
    cli::Parser parser(argc, argv);

    configure_parser(parser);

    parse_command_line(parser);

    QDir route_dir = QDir(route_path.c_str());
    if (!route_dir.exists())
    {
        LOG_WARN("Warn: fail to find route %s", route_path.c_str());
        return false;
    }

    if (!topology->load(route_dir.dirName(), false))
    {
        LOG_WARN("Warn: fail to load topology from route %s", route_path.c_str());
        return false;
    }
    LOG_INFO("Info: load topology from route %s", route_path.c_str());

    traj_list_t *traj_list = topology->getTrajectoriesList();

    for (auto traj = traj_list->begin(); traj != traj_list->end(); ++traj)
    {
        find_ends_without_connector((*traj));
    }

    for (auto traj = traj_list->begin(); traj != traj_list->end(); ++traj)
    {
        check_trajectory((*traj));
    }

    conn_list_t *switches = topology->getConnectorsList();
    for (auto conn = switches->begin(); conn != switches->end(); ++conn)
    {
        Switch *sw = dynamic_cast<Switch *>(*conn);
        if (sw)
        {
            check_connector_point(sw);
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopologyCheck::configure_parser(cli::Parser &parser)
{
    parser.set_optional<std::string>("r", "route",
                                     "",
                                     "Input RRS route path");
    parser.enable_help();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopologyCheck::parse_command_line(cli::Parser &parser)
{
    parser.run_and_exit_if_error();

    cmd_line_t cmd_line;
    cmd_line.route_path = parser.get<std::string>("r");

    if (cmd_line.route_path.isPresent())
    {
        route_path = cmd_line.route_path.value;
        return;
    }

    LOG_WARN("ERROR: Missing route path");
    exit(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopologyCheck::find_ends_without_connector(Trajectory *traj)
{
    // TODO
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopologyCheck::check_trajectory(Trajectory *traj)
{
    // TODO
    LOG_INFO("Info: Check trajectory %s", traj->getName().toStdString().c_str());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopologyCheck::check_connector_point(Switch *sw)
{
    // TODO
    LOG_INFO("Info: Check connector %s", sw->getName().toStdString().c_str());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TopologyCheck::calcCurvature(const track_t &track0, const track_t &track1)
{
    // Направление первого трека
    double A0 = track0.orth.x;
    double B0 = track0.orth.y;

    // Направление второго трека
    double A1 = track1.orth.x;
    double B1 = track1.orth.y;

    double det = A0*B1 - A1*B0;

    // Если треки параллельны - кривизна нулевая
    if ( qAbs(det) < 1e-5 )
    {
        return 0.0;
    }

    // Центр первого трека
    dvec3 S0 = - track0.orth * 0.5 * track0.len;
    double D0 = A0 * S0.x + B0 * S0.y;

    // Центр второго трека
    dvec3 S1 = track1.orth * 0.5 * track1.len;
    double D1 = A1 * S1.x + B1 * S1.y;

    double xC = (B0*D1 - B1*D0) / det;
    double yC = (A0*D1 - A1*D0) / det;

    double rho = std::sqrt(xC * xC + yC * yC);

    double curvature = 1 / rho;

    return curvature;
}
