#include    "converter.h"

#include    <iostream>
#include    <algorithm>
#include    <sstream>
#include    <codecvt>
#include    <cstdlib>
#include    <locale>

#include    "path-utils.h"

#include    <QFile>
#include    <QTextCodec>
#include    <QTextDecoder>
#include    <QTextStream>

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProfConverter::load(const std::string &path,
                         std::vector<track_t> &track_data)
{
    if (path.empty())
        return false;

    std::ifstream stream(path.c_str(), std::ios::in);

    if (!stream.is_open())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    return load(stream, track_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string delete_symbol(const std::string &str, char symbol)
{
    std::string tmp = str;
    tmp.erase(std::remove(tmp.begin(), tmp.end(), symbol), tmp.end());
    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string getLine(std::istream &stream)
{
    std::string line = "";
    std::getline(stream, line);
    std::string tmp = delete_symbol(line, '\r');
    tmp = delete_symbol(tmp, ';');
    std::replace(tmp.begin(), tmp.end(), ',', ' ');

    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProfConverter::load(std::ifstream &stream,
                         std::vector<track_t> &track_data)
{
    std::vector<track_t> tmp_data;

    float length = 0.0f;

    while (!stream.eof())
    {
        std::string line = getLine(stream);

        std::istringstream ss(line);

        track_t track;

        ss >> track.begin_point.x
           >> track.begin_point.y
           >> track.begin_point.z
           >> track.end_point.x
           >> track.end_point.y
           >> track.end_point.z
           >> track.prev_uid
           >> track.next_uid
           >> track.arrows
           >> track.voltage
           >> track.ordinate;

        Vec3 dir_vector = track.end_point - track.begin_point;
        track.length = dir_vector.length();
        track.orth = dir_vector.orth();

        tmp_data.push_back(track);
    }

    track_data.push_back(*tmp_data.begin());
    length += (*tmp_data.begin()).length;

    float rail_coord = 0.0f;

    for (auto it = tmp_data.begin(); (*it).next_uid != -2; ++it)
    {
        track_t cur_track = *it;
        track_t next_track = tmp_data.at(static_cast<size_t>(cur_track.next_uid - 1));
        track_data[track_data.size() - 1].end_point = next_track.begin_point;
        length += next_track.length;

        rail_coord += track_data.at(track_data.size() - 1).length;
        next_track.rail_coord = rail_coord;

        track_data.push_back(next_track);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProfConverter::readWaypoints(const std::string &path,
                                  std::vector<waypoint_t> &waypoints)
{
    if (path.empty())
        return false;

    fileToUtf8(path);

    std::wifstream stream(path.c_str(), std::ios::in);

    stream.imbue(std::locale("ru_RU.UTF-8"));

    if (!stream.is_open())
    {
        std::cout << "File " << path << " not opened" << std::endl;
        return false;
    }

    return readWaypoints(stream, waypoints);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProfConverter::readWaypoints(std::wifstream &stream,
                                  std::vector<waypoint_t> &waypoints)
{
    while (!stream.eof())
    {
        std::wstring line = L"";
        std::getline(stream, line);

        if (line.empty())
            continue;

        std::wistringstream ss(line);

        waypoint_t waypoint;

        ss >> waypoint.name >> waypoint.begin_track >> waypoint.end_track;

        waypoints.push_back(waypoint);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProfConverter::writeWaypoints(const std::string &filename,
                                   std::vector<waypoint_t> &waypoints)
{
    std::string path = compinePath(toNativeSeparators(routeDir), filename);

    QFile file(QString(path.c_str()));

    if (!file.open(QIODevice::WriteOnly))
        return;

    QTextStream stream(&file);

    for (auto it = waypoints.begin(); it != waypoints.end(); ++it)
    {
        track_t track = tracks_data1[(*it).begin_track];
        float coord = track.rail_coord;

        stream << QString::fromStdWString((*it).name) << " " << coord << "\n";
    }

    file.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProfConverter::conversion(const std::string &routeDir)
{
    std::string trk1_path = compinePath(routeDir, "route1.trk");
    std::string trk2_path = compinePath(routeDir, "route2.trk");

    if (load(trk1_path, tracks_data1))
        writeProfileData(tracks_data1, "profile1.conf");

    if (load(trk2_path, tracks_data2))
         writeProfileData(tracks_data2, "profile2.conf");

    if (readWaypoints(compinePath(routeDir, "start_kilometers.dat"), waypoints))
        writeWaypoints("waypoints.conf", waypoints);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProfConverter::writeProfileData(const std::vector<track_t> &tracks_data,
                                     const std::string &file_name)
{
    std::string path = compinePath(toNativeSeparators(routeDir), file_name);
    std::ofstream stream(path.c_str(), std::ios::out);

    for (auto it = tracks_data.begin(); it != tracks_data.end(); ++it)
    {
        track_t track = *it;

        stream << track.rail_coord / 1000.0f << " "
               << track.orth.z * 1000.0f << " "
               << "0.0" << std::endl;
    }

    stream.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProfConverter::fileToUtf8(const std::string &path)
{
    QFile file(QString(path.c_str()));
    QString new_data = "";

    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    QTextCodec *codec_1251 = QTextCodec::codecForName("Windows-1251");
    new_data = codec_1251->toUnicode(data);

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



