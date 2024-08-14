#ifndef     APP_H
#define     APP_H

#include    <QCoreApplication>
#include    <QCommandLineParser>

#include    <zds-track.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Application : public QCoreApplication
{
public:

    Application(int argc, char *argv[]);

    ~Application();

    int exec();

private:

    enum CommandLineResult
    {
        CommandLineOK,
        CommandLineHelp,
        CommandLineVersion,
        CommandLineError
    };

    QCommandLineParser parser;

    QString traj_path = "";

    CommandLineResult parseCommandLine(QString &route_dir,
                                       QString &errorMessage);

    void generate_topology(const QString &route_dir);

    void create_directories(const QString &route_dir);
    void read_zds_tracks(std::vector<zds_track_t> &zds_tracks, QString full_path);
};

#endif
