#ifndef     APP_H
#define     APP_H

#include    <QCoreApplication>
#include    <QCommandLineParser>

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

    CommandLineResult parseCommandLine(QString &route_dir,
                                       QString &errorMessage);
};

#endif
