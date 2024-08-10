#include    "converter.h"

#include    <cstdlib>

#include    "path-utils.h"

#include    <QFile>
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
    std::string branch_traj_prefix1 = "route1_branch_";
    std::string branch_traj_prefix2 = "route2_branch_";
    std::string branch_traj_ext = ".trj";

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
        //readRouteMAP(map_path, neutral_insertions);

        readSpeedsDAT(speeds1_path, speeds_data1);
        readSvetoforDAT(signals1_path, signals_data1);
        readBranchTracksDAT(branch1_path, branch_track_data1, dir);

        writeMainTrajectory(traj_file1, tracks_data1);
        writeSpeeds(speeds1_file, speeds_data1);
    }

    if (is_1 && is_2)
    {
        int dir = -1;
        // Создание profile.conf отключено, симулятор сам читает route2.trk
        //writeProfileData(tracks_data2, "profile2.conf");

        readSpeedsDAT(speeds2_path, speeds_data2);
        readSvetoforDAT(signals2_path, signals_data2);
        readBranchTracksDAT(branch2_path, branch_track_data2, dir);

        writeMainTrajectory(traj_file2, tracks_data2);
        writeSpeeds(speeds2_file, speeds_data2);
    }

    if (readStartKilometersDAT(start_km_path, start_km_data))
    {
        writeWaypoints(waypoints_file, start_km_data);
        writeStations(stations_file, start_km_data);
    }

    if (!branch_track_data1.empty())
    {
        size_t i = 0;
        for (auto it = branch_track_data1.begin(); it != branch_track_data1.end(); ++it)
        {
            ++i;
            zds_branch_track_t *branch_track = *it;
            calcBranchTrack1(branch_track);
            writeBranchTrajectory(branch_traj_prefix1 +
                                  QString("%1").arg(i,3,10,QChar('0')).toStdString() +
                                  branch_traj_ext.c_str(), branch_track);
        }
    }

    if (!branch_track_data2.empty())
    {
        size_t i = 0;
        for (auto it = branch_track_data2.begin(); it != branch_track_data2.end(); ++it)
        {
            ++i;
            zds_branch_track_t *branch_track = *it;
            calcBranchTrack2(branch_track);
            writeBranchTrajectory(branch_traj_prefix2 +
                                  QString("%1").arg(i,3,10,QChar('0')).toStdString() +
                                  branch_traj_ext.c_str(), branch_track);
        }
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
        coord = result.trajectory_coord + tau;
        return result;
    }

    if (id_with_min_distance > 0)
    {
        result = tracks_data[id_with_min_distance - 1];
        rho = point - result.begin_point;
        tau = dot(rho, result.orth);
        if (tau <= result.length)
        {
            coord = result.trajectory_coord + tau;
            return result;
        }
    }

    result = tracks_data[id_with_min_distance];
    coord = result.trajectory_coord;
    return result;
}
