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
#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AppCore::AppCore(int &argc, char **argv) : QCoreApplication(argc, argv)
  , model(nullptr)
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

    QString cmd_line = "";
    for (QString s : this->arguments())
        cmd_line += " " + s;

    Journal::instance()->info("Process " + APPLICATION_NAME + " started with command line: " + cmd_line);
    Journal::instance()->info("Started " + APPLICATION_NAME + " initialization");

    switch (parseCommandLine(parser, command_line, errorMessage))
    {
    case CommandLineOk:

        // Creation and initialization of train model
        model = new Model();
        Journal::instance()->info(QString("Created Model object at address: 0x%1").arg(reinterpret_cast<quint64>(model), 0, 16));
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
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int AppCore::exec()
{
    if (model != Q_NULLPTR)
        model->start();

    return QCoreApplication::exec();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AppCore::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QCoreApplication::notify(receiver, event);

    }
    catch (std::exception &ex)
    {
        QString msg = QString("Error %1 sending event %2 to object %4 (%4)")
                .arg(ex.what())
                .arg(typeid (*event).name())
                .arg(qPrintable(receiver->objectName()))
                .arg(typeid (*receiver).name());

        Journal::instance()->critical(msg);
    }
    catch (...)
    {
        QString msg = QString("Error %1 sending event %2 to object %4 (%4)")
                .arg("<unknown>")
                .arg(typeid (*event).name())
                .arg(qPrintable(receiver->objectName()))
                .arg(typeid (*receiver).name());

        Journal::instance()->critical(msg);
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CommandLineParesrResult AppCore::parseCommandLine(QCommandLineParser &parser,
                                                  simulator_command_line_t &command_line,
                                                  QString &errorMessage)
{
    QCommandLineOption help = parser.addHelpOption();
    QCommandLineOption version = parser.addVersionOption();

    // Train config file
    QCommandLineOption trainConfig(QStringList() << "t" << "train-config",
                                   QCoreApplication::translate("main", "Train configuration"),
                                   QCoreApplication::translate("main", "train-config-file"));

    parser.addOption(trainConfig);

    // Route directory
    QCommandLineOption routeDir(QStringList() << "r" << "route",
                                QCoreApplication::translate("main", "Route directory"),
                                QCoreApplication::translate("main", "route-directory"));

    parser.addOption(routeDir);

    // Clear simulator log
    QCommandLineOption clearLog(QStringList() << "c" << "clear-log",
                                QCoreApplication::translate("main", "Clear simulator's log"));

    parser.addOption(clearLog);

    // Allow debug print
    QCommandLineOption debugPrint(QStringList() << "o" << "debug-print",
                                  QCoreApplication::translate("main", "Allow debug print"));

    parser.addOption(debugPrint);    

    QCommandLineOption initCoord(QStringList() << "x" << "init-coord",
                                 QCoreApplication::translate("main", "Initial railway coordinate"),
                                 QCoreApplication::translate("main", "init-coord"));

    parser.addOption(initCoord);

    QCommandLineOption direction(QStringList() << "d" << "direction",
                                 QCoreApplication::translate("main", "Motion's direction"),
                                 QCoreApplication::translate("main", "direction"));    

    parser.addOption(direction);

    QCommandLineOption trajectory_name(QStringList() << "p" << "traj-name",
                                       QCoreApplication::translate("main", "Initial trajectory name"),
                                       QCoreApplication::translate("main", "traj-name"));

    parser.addOption(trajectory_name);

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

    if (parser.isSet(routeDir))
    {
        command_line.route_dir.is_present = true;
        command_line.route_dir.value = parser.value(routeDir);
    }

    if (parser.isSet(clearLog))
    {
        command_line.clear_log.is_present = command_line.clear_log.value = true;
    }

    if (parser.isSet(debugPrint))
    {
        command_line.debug_print.is_present = command_line.debug_print.value = true;
    }

    if (parser.isSet(initCoord))
    {
        command_line.init_coord.is_present = true;
        QString tmp = parser.value(initCoord);                
        command_line.init_coord.value = tmp.toDouble();
    }

    if (parser.isSet(direction))
    {
        command_line.direction.is_present = true;
        QString tmp = parser.value(direction);
        command_line.direction.value = tmp.toInt();
    }

    if (parser.isSet(trajectory_name))
    {
        command_line.trajectory_name.is_present = true;
        command_line.trajectory_name.value = parser.value(trajectory_name);
    }

    return CommandLineOk;
}
