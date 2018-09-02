//------------------------------------------------------------------------------
//
//      Main application class
//      (c) maisvendoo, 01/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Main application class
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 01/09/2018
 */

#include    "app.h"
#include    "CfgReader.h"
#include    "global-const.h"

#include    "filesystem.h"

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
    // Common application data settings
    this->setApplicationName(APPLICATION_NAME);
    this->setApplicationVersion(APPLICATION_VERSION);

    QString errorMessage = "";

    switch (parseCommandLine(parser, command_line, errorMessage))
    {
    case CommandLineOk:

        // Creation and initialization of train model
        model = new Model();
        return model->init(command_line);

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

    // Train config file
    QCommandLineOption trainConfig(QStringList() << "t" << "train-config",
                                   QCoreApplication::translate("main", "Train configuration"),
                                   QCoreApplication::translate("main", "train-config-file"));

    parser.addOption(trainConfig);

    // Parse command line arguments
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

    if (parser.isSet(trainConfig))
    {
        command_line.train_config.is_present = true;
        command_line.train_config.value = parser.value(trainConfig);
    }

    return CommandLineOk;
}
