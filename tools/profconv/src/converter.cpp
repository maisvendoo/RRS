#include    "converter.h"
#include    "command-line.h"
#include    "path-utils.h"
#include    "Logger.h"

#include    <cstdlib>

#include    <QFile>
#include    <QDir>
#include    <QStringConverter>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ZDSimConverter::ZDSimConverter()
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
    cli::Parser parser(argc, argv);

    configure_parser(parser);

    parse_command_line(parser);

    bool result = conversion();

    return result;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::configure_parser(cli::Parser &parser)
{
    parser.set_optional<std::string>("r", "route",
                                     "",
                                     "Input ZDS route path, used as output too");

    parser.set_optional<std::string>("i", "input-route",
                                     "",
                                     "Input ZDS route path");

    parser.set_optional<std::string>("o", "output-route",
                                     "",
                                     "Output RRS route path");

    parser.enable_help();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::parse_command_line(cli::Parser &parser)
{
    parser.run_and_exit_if_error();

    cmd_line_t cmd_line;
    cmd_line.route_path = parser.get<std::string>("r");
    cmd_line.input_route_path = parser.get<std::string>("i");
    cmd_line.output_route_path = parser.get<std::string>("o");

    if (cmd_line.route_path.isPresent())
    {
        configure_path(cmd_line.route_path.value, cmd_line.route_path.value);
        return;
    }

    if (cmd_line.input_route_path.isPresent() && cmd_line.output_route_path.isPresent())
    {
        configure_path(cmd_line.input_route_path.value, cmd_line.output_route_path.value);
        return;
    }

    LOG_WARN("ERROR: Missing route path");
    exit(0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ZDSimConverter::configure_path(const std::string &input_path, const std::string &output_path)
{
    ZDSrouteDir = input_path;
    routeDir = output_path;

    QDir route(routeDir.c_str());

    topologyDir = toNativeSeparators(compinePath(routeDir, DIR_TOPOLOGY));
    if (!route.exists(DIR_TOPOLOGY.c_str()))
    {
        route.mkpath(topologyDir.c_str());
    }
    QDir topology(topologyDir.c_str());

    // Бэкап траекторий
    if (topology.exists(DIR_TRAJECTORIES.c_str()))
    {
        std::string backup = FILE_BACKUP_PREFIX + DIR_TRAJECTORIES + FILE_BACKUP_EXTENTION;
        if (topology.exists(backup.c_str()))
        {
            std::string old_backup_path = compinePath(topologyDir, backup);
            QDir(old_backup_path.c_str()).removeRecursively();
        }
        topology.rename(DIR_TRAJECTORIES.c_str(), backup.c_str());
    }
    trajectoriesDir = toNativeSeparators(compinePath(topologyDir, DIR_TRAJECTORIES));
    topology.mkpath(trajectoriesDir.c_str());

    // Бэкап расстановки объектов
    if (topology.exists(DIR_ROUTE1MAP.c_str()))
    {
        std::string backup = FILE_BACKUP_PREFIX + DIR_ROUTE1MAP + FILE_BACKUP_EXTENTION;
        if (topology.exists(backup.c_str()))
        {
            std::string old_backup_path = compinePath(topologyDir, backup);
            QDir(old_backup_path.c_str()).removeRecursively();
        }
        topology.rename(DIR_ROUTE1MAP.c_str(), backup.c_str());
    }
    route1mapDir = toNativeSeparators(compinePath(topologyDir, DIR_ROUTE1MAP));
    topology.mkpath(route1mapDir.c_str());

    // Бэкап карты АЛСН
    if (topology.exists(DIR_ALSN_MAP.c_str()))
    {
        std::string backup = FILE_BACKUP_PREFIX + DIR_ALSN_MAP + FILE_BACKUP_EXTENTION;
        if (topology.exists(backup.c_str()))
        {
            std::string old_backup_path = compinePath(topologyDir, backup);
            QDir(old_backup_path.c_str()).removeRecursively();
        }
        topology.rename(DIR_ALSN_MAP.c_str(), backup.c_str());
    }
    ALSN_Dir = toNativeSeparators(compinePath(topologyDir, DIR_ALSN_MAP));
    topology.mkpath(ALSN_Dir.c_str());

    // Бэкап карты скоростей
    if (topology.exists(DIR_SPEEDMAP.c_str()))
    {
        std::string backup = FILE_BACKUP_PREFIX + DIR_SPEEDMAP + FILE_BACKUP_EXTENTION;
        if (topology.exists(backup.c_str()))
        {
            std::string old_backup_path = compinePath(topologyDir, backup);
            QDir(old_backup_path.c_str()).removeRecursively();
        }
        topology.rename(DIR_SPEEDMAP.c_str(), backup.c_str());
    }
    speedmapDir = toNativeSeparators(compinePath(topologyDir, DIR_SPEEDMAP));
    topology.mkpath(speedmapDir.c_str());
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
bool ZDSimConverter::conversion()
{
    std::string trk1_path = compinePath(ZDSrouteDir, "route1.trk");
    std::string trk2_path = compinePath(ZDSrouteDir, "route2.trk");
    std::string objects_path = compinePath(ZDSrouteDir, "objects.ref");
    std::string map_path = compinePath(ZDSrouteDir, "route1.map");
    std::string start_km_path = compinePath(ZDSrouteDir, "start_kilometers.dat");
    std::string speeds1_path = compinePath(ZDSrouteDir, "speeds1.dat");
    std::string speeds2_path = compinePath(ZDSrouteDir, "speeds2.dat");
    std::string signals1_path = compinePath(ZDSrouteDir, "svetofor1.dat");
    std::string signals2_path = compinePath(ZDSrouteDir, "svetofor2.dat");
    std::string branch1_path = compinePath(ZDSrouteDir, "branch_tracks1.dat");
    std::string branch2_path = compinePath(ZDSrouteDir, "branch_tracks2.dat");

/*  Устаревший формат
    std::string traj_file1 = "route1.trj";
    std::string traj_file2 = "route2.trj";

    std::string waypoints_file = "waypoints.conf";
    std::string stations_file = "stations.conf";
    std::string speeds1_file = "speeds1.conf";
    std::string speeds2_file = "speeds2.conf";
*/
    // Костыль - создаваемые специально для конвертации файлы
    // со светофорами по главным путям в неправильном направлении
    std::string signals1_reverse_path = compinePath(ZDSrouteDir, "svetofor1_reverse.dat");
    std::string signals2_reverse_path = compinePath(ZDSrouteDir, "svetofor2_reverse.dat");

    int dir = 1;
    bool is_1 = readRouteTRK(trk1_path, tracks_data1, dir);
    dir = -1;
    bool is_2 = readRouteTRK(trk2_path, tracks_data2, dir);

    if (is_1)
    {
        dir = 1;
        // Создание profile.conf отключено, симулятор сам читает route1.trk
        //writeProfileData(tracks_data1, "profile1.conf");

        //createPowerLine(tracks_data1, power_line1);
        readObjectsRef(objects_path, objects_data);
        readRouteMAP(map_path, route_map_data);
        findNeutralInsertions(neutral_insertions);

        readSpeedsDAT(speeds1_path, speeds_data1, dir);
        readSvetoforDAT(signals1_path, signals_data1, dir);
        readSvetoforDAT(signals1_reverse_path, signals_reverse_data1, dir);
        readBranchTracksDAT(branch1_path, dir);

        // Создание старого формата ЭК скоростей speeds1.conf отключено
        //writeOldSpeeds(speeds1_file, speeds_data1);
    }

    if (is_1 && is_2)
    {
        dir = -1;
        // Создание profile.conf отключено, симулятор сам читает route2.trk
        //writeProfileData(tracks_data2, "profile2.conf");

        readSpeedsDAT(speeds2_path, speeds_data2, dir);
        readSvetoforDAT(signals2_path, signals_data2, dir);
        readSvetoforDAT(signals2_reverse_path, signals_reverse_data2, dir);
        readBranchTracksDAT(branch2_path, dir);

        // Создание старого формата ЭК скоростей speeds2.conf отключено
        //writeOldSpeeds(speeds2_file, speeds_data2);
    }


    if (is_1)
    {
        findSignalsAtMap();

        findSplitsMainTrajectory1();

        if (is_2)
            findSplitsMainTrajectory2();
    }

    // Разделение главных путей на подтраектории
    if (is_1)
    {
        dir = 1;
        std::sort(split_data1.begin(), split_data1.end(), split_zds_trajectory_t::compare_by_track_id);
        splitMainTrajectory(dir);
        for (auto traj = trajectories1.begin(); traj != trajectories1.end(); ++traj)
        {
            writeTopologyTrajectory(*traj);
        }
    }
    if (is_1 && is_2)
    {
        dir = -1;
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

        dir = 1;
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

        dir = -1;
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

        dir = 1;
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

        dir = -1;
        size_t num_trajectories = 0;
        for (auto it = branch_2plus2_data.begin(); it != branch_2plus2_data.end(); ++it)
        {
            ++num_trajectories;
            nameBranch22(*it, dir, num_trajectories);
            writeTopologyTrajectory(&((*it)->trajectory));
        }
    }
/*
    // Отладка поиска светофоров, расставленных на карте
    writeSignalsForDebug();

    // Отладка разделения путей на подтраектории
    dir = 0;
    writeSplitsForDebug(branch_connectors, dir);
    dir = 1;
    writeSplitsForDebug(split_data1, dir);
    dir = -1;
    writeSplitsForDebug(split_data2, dir);
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

    writeMap();

    writeModelsConfig();

    writeALSN();

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ZDSimConverter::calcCurvature(const zds_trajectory_data_t &tracks_data, size_t idx)
{
    if ((idx == 0) || (idx >= (tracks_data.size() - 1)))
    {
        return 0.0;
    }

    zds_track_t track0 = tracks_data[idx - 1];
    zds_track_t track1 = tracks_data[idx];

    return calcCurvature(track0, track1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double ZDSimConverter::calcCurvature(const zds_track_t &track0, const zds_track_t &track1)
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
    dvec3 S0 = - track0.orth * 0.5 * track0.length;
    double D0 = A0 * S0.x + B0 * S0.y;

    // Центр второго трека
    dvec3 S1 = track1.orth * 0.5 * track1.length;
    double D1 = A1 * S1.x + B1 * S1.y;

    double xC = (B0*D1 - B1*D0) / det;
    double yC = (A0*D1 - A1*D0) / det;

    double rho = std::sqrt(xC * xC + yC * yC);

    double curvature = 1 / rho;

    return curvature;
}
