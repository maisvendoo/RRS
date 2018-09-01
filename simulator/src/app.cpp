#include    "app.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AppCore::AppCore(int &argc, char **argv) : QCoreApplication(argc, argv)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AppCore::~AppCore()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AppCore::init()
{
    QString errorMessage = "";
    switch (parseCommandLine(parser, command_line, errorMessage))
    {
    case CommandLineOk:

        // Here will be application and train model initialization
        return true;

    case CommandLineError:

        fputs(qPrintable(errorMessage), stderr);
        fputs("\n", stderr);
        return false;

    case CommandLineVersionRequired:

        printf("%s %s\n",
               qPrintable(this->applicationName()),
               qPrintable(this->applicationVersion()));

        return false;

    case CommandLineHelpRequired:

        parser.showHelp();
        return false;

    default:

        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int AppCore::exec()
{
    return QCoreApplication::exec();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CommandLineParesrResult AppCore::parseCommandLine(QCommandLineParser &parser,
                                                  command_line_t &command_line,
                                                  QString &errorMessage)
{
    QCommandLineOption help = parser.addHelpOption();
    QCommandLineOption version = parser.addVersionOption();

    if (!parser.parse(this->arguments()))
    {
        errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(version))
    {
        return CommandLineVersionRequired;
    }

    if (parser.isSet(help))
    {
        return CommandLineHelpRequired;
    }

    return CommandLineOk;
}
