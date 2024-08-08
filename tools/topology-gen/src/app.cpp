#include    <app.h>

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
