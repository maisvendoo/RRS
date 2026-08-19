#ifndef     APP_H
#define     APP_H

#include    <QCoreApplication>
#include    <QCommandLineParser>
#include    <QMap>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum CommandLineResult
{
    CommandLineOK,
    CommandLineHelp,
    CommandLineVersion,
    CommandLineError
};

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

    QCommandLineParser  parser;



    CommandLineResult parseCommandLine(QString &routeDir, QString &errorMessage);
};

#endif // APP_H
