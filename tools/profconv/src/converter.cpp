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
void ZDSimConverter::fileToUtf8(const std::string &path)
{
    QFile file(QString(path.c_str()));
    QString new_data = "";

    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    auto toUtf8 = QStringDecoder(QStringConverter::Utf8);
    new_data = toUtf8(data);
    file.close();

    QFile::rename(QString(compinePath(routeDir, "start_kilometers.dat").c_str()),
                  compinePath(routeDir, "start_kilometers.dat.bak").c_str());

    QFile startKmDat(compinePath(routeDir, "start_kilometers.dat").c_str());

    if (!startKmDat.open(QIODevice::WriteOnly))
        return;

    QTextStream stream(&startKmDat);

    stream << new_data;

    startKmDat.close();
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

    std::string waypoints_file = "waypoints.conf";
    std::string stations_file = "stations.conf";
    std::string speeds1_file = "speeds1.conf";
    std::string speeds2_file = "speeds2.conf";

    if (readRouteTRK(trk1_path, tracks_data1))
    {
        // Создание profile.conf отключено, симулятор сам читает route1.trk
        //writeProfileData(tracks_data1, "profile1.conf");
        createPowerLine(tracks_data1, power_line1);
        readRouteMAP(map_path, neutral_insertions);

        readSpeedsDAT(speeds1_path, speeds_data1);
        readSvetoforDAT(signals1_path, signals_data1);
        readBranchTracksDAT(branch1_path, branch_track_data1);

        writeSpeeds(speeds1_file, speeds_data1);
    }

    if (readRouteTRK(trk2_path, tracks_data2))
    {
        // Создание profile.conf отключено, симулятор сам читает route2.trk
        //writeProfileData(tracks_data2, "profile2.conf");

        readSpeedsDAT(speeds2_path, speeds_data2);
        readSvetoforDAT(signals2_path, signals_data2);
        readBranchTracksDAT(branch2_path, branch_track_data2);

        writeSpeeds(speeds2_file, speeds_data2);
    }

    if (readStartKilometersDAT(start_km_path, start_km_data))
    {
        writeWaypoints(waypoints_file, start_km_data);
        writeStations(stations_file, start_km_data);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
zds_track_t ZDSimConverter::getNearestTrack(dvec3 point, const zds_trajectory_data_t &tracks_data, float &coord)
{
    zds_track_t result;

    for (auto it = tracks_data.begin(); it != tracks_data.end(); ++it)
    {
        zds_track_t track = *it;

        dvec3 rho = point - track.begin_point;
        double tau = dot(rho, track.orth);

        if ( tau < 0.0 )
            continue;

        if ( tau > track.length )
            continue;

        result = track;
        coord = track.trajectory_coord + tau;

        break;
    }    

    return result;
}
