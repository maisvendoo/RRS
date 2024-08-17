#ifndef     APP_H
#define     APP_H

#include    <QCoreApplication>
#include    <QCommandLineParser>

#include    <zds-track.h>
#define     DELIMITER_SYMBOL char('\t')
#define     FILE_EXTENTION   std::string(".traj")
#define     FILE_BACKUP_EXTENTION std::string(".bak")

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

    QString routeDir = "";
    QString filename = "";
    QString trkfile = "";
    int begin_track = 0;
    int end_track = 1;
    double bias = 7.5;

    QString trajDir = "";

    CommandLineResult parseCommandLine(QString &errorMessage);

    void generate_topology(const QString &route_dir);

    void create_directories(const QString &route_dir);
    void read_zds_tracks(std::vector<zds_track_t> &zds_tracks, QString full_path);
};

#endif
