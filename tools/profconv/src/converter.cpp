#include    "converter.h"

#include    <cstdlib>

#include    "path-utils.h"

#include    <QFile>
#include    <QDir>
#include    <QStringConverter>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ZDSimConverter::ZDSimConverter()
    : routeDir("")
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ZDSimConverter::~ZDSimConverter()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int ZDSimConverter::run(int argc, char *argv[])
{
    switch (parseCommandLine(argc, argv))
    {
    case RESULT_OK:

        if ( !conversion(toNativeSeparators(routeDir)) )
            return -1;

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
CmdLineParseResult ZDSimConverter::parseCommandLine(int argc, char *argv[])
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

    std::string topology_subdir_name = "topology";
    topologyDir = compinePath(routeDir, topology_subdir_name);

    QDir route(routeDir.c_str());
    if (!route.exists(topology_subdir_name.c_str()))
    {
        route.mkpath(topologyDir.c_str());
    }

    std::string trajectories_subdir_name = "trajectories";
    trajectoriesDir = compinePath(topologyDir, trajectories_subdir_name);

    QDir topology(topologyDir.c_str());
    if (!topology.exists(trajectories_subdir_name.c_str()))
    {
        topology.mkpath(trajectoriesDir.c_str());
    }

    return RESULT_OK;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString ZDSimConverter::fileToQString(const std::string &path)
{
    QString file_path = QString(path.c_str());
    QFile file(file_path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return "";
    }

    QByteArray data = file.readAll();
    auto toUtf8 = QStringDecoder(QStringConverter::System);
    QString new_data = toUtf8(data);
//    QString new_data(data);
    file.close();

    return new_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ZDSimConverter::conversion(const std::string &routeDir)
{
    std::string trk1_path = compinePath(routeDir, "route1.trk");
    std::string trk2_path = compinePath(routeDir, "route2.trk");
    std::string map_path = compinePath(routeDir, "route1.map");
    std::string start_km_path = compinePath(routeDir, "start_kilometers.dat");
    std::string speeds1_path = compinePath(routeDir, "speeds1.dat");
    std::string speeds2_path = compinePath(routeDir, "speeds2.dat");
    std::string signals1_path = compinePath(routeDir, "svetofor1.dat");
    std::string signals2_path = compinePath(routeDir, "svetofor2.dat");
    std::string branch1_path = compinePath(routeDir, "branch_tracks1.dat");
    std::string branch2_path = compinePath(routeDir, "branch_tracks2.dat");

    std::string traj_file1 = "route1.trj";
    std::string traj_file2 = "route2.trj";

    std::string waypoints_file = "waypoints.conf";
    std::string stations_file = "stations.conf";
    std::string speeds1_file = "speeds1.conf";
    std::string speeds2_file = "speeds2.conf";

    bool is_1 = readRouteTRK(trk1_path, tracks_data1);
    bool is_2 = readRouteTRK(trk2_path, tracks_data2);

    if (is_1)
    {
        int dir = 1;
        // Создание profile.conf отключено, симулятор сам читает route1.trk
        //writeProfileData(tracks_data1, "profile1.conf");

        //createPowerLine(tracks_data1, power_line1);
        readRouteMAP(map_path, route_map_data);
        findNeutralInsertions(neutral_insertions);

        readSpeedsDAT(speeds1_path, speeds_data1, dir);
        readSvetoforDAT(signals1_path, signals_data1, dir);
        readBranchTracksDAT(branch1_path, dir);

        // Создание старого формата ЭК скоростей speeds1.conf отключено
        //writeOldSpeeds(speeds1_file, speeds_data1);
    }

    if (is_1 && is_2)
    {
        int dir = -1;
        // Создание profile.conf отключено, симулятор сам читает route2.trk
        //writeProfileData(tracks_data2, "profile2.conf");

        readSpeedsDAT(speeds2_path, speeds_data2, dir);
        readSvetoforDAT(signals2_path, signals_data2, dir);
        readBranchTracksDAT(branch2_path, dir);

        // Создание старого формата ЭК скоростей speeds2.conf отключено
        //writeOldSpeeds(speeds2_file, speeds_data2);
    }

    if (is_1)
    {
        int dir = 1;
        findSplitsMainTrajectories(dir);
        // Отладка
        //writeMainTrajectory(traj_file1, tracks_data1);
    }

    if (is_1 && is_2)
    {
        int dir = -1;
        findSplitsMainTrajectories(dir);
        // Отладка
        //writeMainTrajectory(traj_file2, tracks_data2);
    }

    // Разделение главных путей на подтраектории
    if (is_1)
    {
        int dir = 1;
        std::sort(split_data1.begin(), split_data1.end(), split_zds_trajectory_t::compare_by_track_id);
        splitMainTrajectory(dir);
        for (auto traj = trajectories1.begin(); traj != trajectories1.end(); ++traj)
        {
            writeTopologyTrajectory(*traj);
        }
    }
    if (is_1 && is_2)
    {
        int dir = -1;
        std::sort(split_data2.begin(), split_data2.end(), split_zds_trajectory_t::compare_by_track_id);
        splitMainTrajectory(dir);
        for (auto traj = trajectories2.begin(); traj != trajectories2.end(); ++traj)
        {
            writeTopologyTrajectory(*traj);
        }
    }

    if (!branch_track_data1.empty())
    {
        std::sort(branch_track_data1.begin(), branch_track_data1.end(), zds_branch_track_t::compare_by_track_id_begin);

        int dir = 1;
        size_t num_trajectories = 0;
        for (auto it = branch_track_data1.begin(); it != branch_track_data1.end(); ++it)
        {
            ++num_trajectories;
            splitAndNameBranch(*it, dir, num_trajectories);
            for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
            {
                writeTopologyTrajectory(*traj);
            }
        }
    }

    if (!branch_track_data2.empty())
    {
        std::sort(branch_track_data2.begin(), branch_track_data2.end(), zds_branch_track_t::compare_by_track_id_end);

        int dir = -1;
        size_t num_trajectories = 0;
        for (auto it = branch_track_data2.begin(); it != branch_track_data2.end(); ++it)
        {
            ++num_trajectories;
            splitAndNameBranch(*it, dir, num_trajectories);
            for (auto traj = (*it)->trajectories.begin(); traj != (*it)->trajectories.end(); ++traj)
            {
                writeTopologyTrajectory(*traj);
            }
        }
    }

    if (!branch_2minus2_data.empty())
    {
        std::sort(branch_2minus2_data.begin(), branch_2minus2_data.end(), zds_branch_2_2_t::compare_by_track_id1);

        int dir = 1;
        size_t num_trajectories = 0;
        for (auto it = branch_2minus2_data.begin(); it != branch_2minus2_data.end(); ++it)
        {
            ++num_trajectories;
            nameBranch22(*it, dir, num_trajectories);
            writeTopologyTrajectory(&((*it)->trajectory));
        }
    }

    if (!branch_2plus2_data.empty())
    {
        std::sort(branch_2plus2_data.begin(), branch_2plus2_data.end(), zds_branch_2_2_t::compare_by_track_id2);

        int dir = -1;
        size_t num_trajectories = 0;
        for (auto it = branch_2plus2_data.begin(); it != branch_2plus2_data.end(); ++it)
        {
            ++num_trajectories;
            nameBranch22(*it, dir, num_trajectories);
            writeTopologyTrajectory(&((*it)->trajectory));
        }
    }
/*
    // Отладка разделения путей на подтраектории
    int dir = 0;
    writeSplits(branch_connectors, dir);
    dir = 1;
    writeSplits(split_data1, dir);
    dir = -1;
    writeSplits(split_data2, dir);
*/
    writeTopologyConnectors();

    if (readStartKilometersDAT(start_km_path, start_km_data))
    {
        // Точки старта у станционных светофоров
        findStartPointsBySignals(split_data1);
        findStartPointsBySignals(split_data2);
        findStartPointsBySignals(branch_connectors);
        //std::sort(start_points.begin(), start_points.end(), start_point_t::compare_by_direction);
        std::sort(start_points.begin(), start_points.end(), start_point_t::compare_by_station_id);
        writeStartPoints(start_points);

        // Создание старого формата ЭК станций stations.conf отключено
        //writeStationsOld(stations_file, start_km_data);

        // Координаты центральных точек станций
        writeStations(start_km_data);
    }

    if (createSpeedMap())
    {
        writeSpeedmap();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
zds_track_t ZDSimConverter::getNearestTrack(dvec3 point, const zds_trajectory_data_t &tracks_data, float &coord)
{
    int id_with_min_distance = 0;
    double min_distance = 1e10;

    size_t id = 0;
    for (auto it = tracks_data.begin(); it != tracks_data.end(); ++it)
    {
        zds_track_t track = *it;

        double distance = length(point - track.begin_point);
        if (distance < min_distance)
        {
            min_distance = distance;
            id_with_min_distance = id;
        }
        ++id;
    }

    zds_track_t result = tracks_data[id_with_min_distance];
    dvec3 rho = point - result.begin_point;
    double tau = dot(rho, result.orth);
    if (tau >= 0.0)
    {
        coord = result.route_coord + tau;
        return result;
    }

    if (id_with_min_distance > 0)
    {
        result = tracks_data[id_with_min_distance - 1];
        rho = point - result.begin_point;
        tau = dot(rho, result.orth);
        if (tau <= result.length)
        {
            coord = result.route_coord + tau;
            return result;
        }
    }

    result = tracks_data[id_with_min_distance];
    coord = result.route_coord;
    return result;
}
