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
        LOG_WARN("Error: fail to find route %s", route_path.c_str());
        return false;
    }

    if (!topology->load(route_dir.dirName(), false))
    {
        LOG_WARN("Error: fail to load topology from route %s", route_path.c_str());
        return false;
    }
    LOG_INFO("Info: load topology from route %s", route_path.c_str());
    LOG_INFO("================================================================================");

    traj_list_t *traj_list = topology->getTrajectoriesList();

    for (auto traj = traj_list->begin(); traj != traj_list->end(); ++traj)
    {
        find_ends_without_connector((*traj));
    }

    for (auto traj = traj_list->begin(); traj != traj_list->end(); ++traj)
    {
        check_trajectory((*traj));
    }
    LOG_INFO("================================================================================");

    conn_list_t *switches = topology->getConnectorsList();
    for (auto conn = switches->begin(); conn != switches->end(); ++conn)
    {
        Switch *sw = dynamic_cast<Switch *>(*conn);
        if (sw)
        {
            check_connector_point(sw);
        }
    }
    LOG_INFO("================================================================================");

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
    parser.set_optional<double>("c", "curve",
                                150.0,
                                "Minimum curve radius, m");
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
    cmd_line.minimum_curve_radius = parser.get<double>("c");

    if (cmd_line.minimum_curve_radius.isPresent())
    {
        maximum_curvature = 1.0 / max(1.0, cmd_line.minimum_curve_radius.value);
    }

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
    if (traj->getBwdConnector() == nullptr)
    {
        dvec3 end_point = traj->getFirstTrack().begin_point;
        ends_without_connector.push_back({end_point, traj});
/*
        LOG_INFO("Info: no connector at begin {%12.3f;%12.3f;%8.3f} of trajectory %s",
                 end_point.x, end_point.y, end_point.z, traj->getName().toStdString().c_str());
*/
    }
    if (traj->getFwdConnector() == nullptr)
    {
        dvec3 end_point = traj->getLastTrack().end_point;
        ends_without_connector.push_back({end_point, traj});
/*
        LOG_INFO("Info: no connector at end {%12.3f;%12.3f;%8.3f} of trajectory %s",
                 end_point.x, end_point.y, end_point.z, traj->getName().toStdString().c_str());
*/
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopologyCheck::check_trajectory(Trajectory *traj)
{
    int point_num = 1;
    for (const auto& track : traj->getTracks())
    {
        if (point_num > 1)
        {
            const auto& prev_track = traj->getTracks().at(point_num - 2);
            double curvature = calcCurvature(prev_track, track);
            if (curvature > maximum_curvature)
            {
                LOG_WARN("Warn: curve at points %u, %u, %u at trajectory %s with very small radius %5.1f m",
                         point_num - 1, point_num, point_num + 1,
                         traj->getName().toStdString().c_str(),
                         1.0 / curvature);
            }
        }

        if (track.len < 0.01)
        {
            LOG_WARN("Warn: points %u {%12.3f;%12.3f;%8.3f} and %u {%12.3f;%12.3f;%8.3f} match in trajectory %s",
                     point_num, track.begin_point.x, track.begin_point.y, track.begin_point.z,
                     point_num + 1, track.end_point.x, track.end_point.y, track.end_point.z,
                     traj->getName().toStdString().c_str());
        }
        check_ends_and_point(traj, track.begin_point, point_num);
        ++point_num;
    }
    check_ends_and_point(traj, traj->getLastTrack().end_point, point_num);

//    LOG_INFO("Info: Check trajectory %s", traj->getName().toStdString().c_str());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopologyCheck::check_connector_point(Switch *sw)
{
    std::array<std::pair<Switch::State, Switch::State>, 4> states;
    states[0] = {Switch::STATE_PLUS, Switch::STATE_PLUS};
    states[1] = {Switch::STATE_PLUS, Switch::STATE_MINUS};
    states[2] = {Switch::STATE_MINUS, Switch::STATE_PLUS};
    states[3] = {Switch::STATE_MINUS, Switch::STATE_MINUS};
    for (auto [state_bwd, state_fwd] : states)
    {
        sw->setStateBwd(state_bwd);
        sw->setStateFwd(state_fwd);
        if (sw->getBwdTraj() && sw->getFwdTraj())
        {
            auto bwd_track = sw->getBwdTraj()->getLastTrack();
            auto fwd_track = sw->getFwdTraj()->getFirstTrack();

            double curvature = calcCurvature(bwd_track, fwd_track);
            if (curvature > maximum_curvature)
            {
                LOG_WARN("Warn: bwd_trajectory %s and fwd_trajectory %s of connector %s create curve with very small radius %5.1f m",
                         sw->getBwdTraj()->getName().toStdString().c_str(),
                         sw->getFwdTraj()->getName().toStdString().c_str(),
                         sw->getName().toStdString().c_str(),
                         1.0 / curvature);
            }

            dvec3 bwd_end = bwd_track.end_point;
            dvec3 fwd_begin = fwd_track.begin_point;
            if (length(bwd_end - fwd_begin) >= 0.01)
            {
                LOG_WARN("Warn: point at end {%12.3f;%12.3f;%8.3f} of trajectory %s is far away from point at begin {%12.3f;%12.3f;%8.3f} of trajectory %s in connector %s",
                         bwd_end.x, bwd_end.y, bwd_end.z,
                         sw->getBwdTraj()->getName().toStdString().c_str(),
                         fwd_begin.x, fwd_begin.y, fwd_begin.z,
                         sw->getFwdTraj()->getName().toStdString().c_str(),
                         sw->getName().toStdString().c_str());
            }
        }
    }
//    LOG_INFO("Info: Check connector %s", sw->getName().toStdString().c_str());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TopologyCheck::check_ends_and_point(Trajectory *traj, dvec3 point, int point_num)
{
    for (auto& [end_point, end_traj] : ends_without_connector)
    {
        if (end_traj == traj)
        {
            continue;
        }

        if (length(end_point - point) < 0.01)
        {
            std::string end_type = "";
            if (length(end_point - end_traj->getFirstTrack().begin_point) < 0.01)
                end_type = "begin";
            if (length(end_point - end_traj->getLastTrack().end_point) < 0.01)
                end_type = "  end";
            LOG_WARN("Warn: point at %s {%12.3f;%12.3f;%8.3f} of trajectory %s match point %u {%12.3f;%12.3f;%8.3f} of trajectory %s but there is no connector",
                     end_type.c_str(), end_point.x, end_point.y, end_point.z,
                     end_traj->getName().toStdString().c_str(),
                     point_num, point.x, point.y, point.z,
                     traj->getName().toStdString().c_str());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double TopologyCheck::calcCurvature(const track_t &track0, const track_t &track1)
{
    if (track0.len < 0.01)
        return 0.0;

    if (track1.len < 0.01)
        return 0.0;

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
