#include    <app.h>

#include    <QDir>
#include    <QDirIterator>
#include    <QStringList>

#include    <fstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Application::Application(int argc, char *argv[])
    : QCoreApplication(argc, argv)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Application::~Application()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Application::exec()
{
    QString routeDir = "";
    QString errorMessage = "";

    switch (parseCommandLine(routeDir, errorMessage))
    {

    case CommandLineOK:
    {
        generate_topology(routeDir);
        return 0;
    }

    case CommandLineHelp:

        parser.showHelp();

    case CommandLineVersion:

        return 0;

    case CommandLineError:

        fputs(qPrintable(errorMessage), stderr);
        fputs("\n", stderr);
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Application::CommandLineResult Application::parseCommandLine(QString &route_dir,
                                                QString &errorMessage)
{
    QCommandLineOption optHelp = parser.addHelpOption();
    QCommandLineOption optVersion = parser.addVersionOption();

    QCommandLineOption optRouteDir(QStringList() << "r" << "route",
                                   QCoreApplication::translate("main", "Path to route's directory"),
                                   QCoreApplication::translate("main", "route directory path"));

    parser.addOption(optRouteDir);

    if (!parser.parse(this->arguments()))
    {
        errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(optHelp))
        return CommandLineHelp;

    if (parser.isSet(optVersion))
        return CommandLineVersion;

    if (parser.isSet(optRouteDir))
    {
        route_dir = parser.value(optRouteDir);
    }

    return CommandLineOK;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::create_directories(const QString &route_dir)
{
    QDir route(route_dir);

    traj_path = "topology" + QString(QDir::separator()) + "trajectories";

    if (!route.exists("topology"))
    {
        route.mkpath(traj_path);
    }
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
void Application::read_zds_tracks(std::vector<zds_track_t> &zds_tracks,
                                  QString full_path)
{
    std::ifstream in_stream(full_path.toStdString(), std::ios::in);

    if (!in_stream.is_open())
    {
        return;
    }

    while (!in_stream.eof())
    {
        std::string line = "";
        line = getLine(in_stream);

        std::istringstream ss(line);

        zds_track_t track;

        ss  >> track.begin_point.x
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

        zds_tracks.push_back(track);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::generate_topology(const QString &route_dir)
{
    create_directories(route_dir);

    QDir route(route_dir);
    QDirIterator trk_files(route.path(), QStringList() << "*.trk",
                           QDir::NoDotAndDotDot | QDir::Files);

    while (trk_files.hasNext())
    {
        QString full_path = trk_files.next();

        std::vector<zds_track_t> zds_tracks;

        read_zds_tracks(zds_tracks, full_path);

        if (zds_tracks.empty())
        {
            continue;
        }
    }
}
