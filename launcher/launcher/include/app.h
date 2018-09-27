//------------------------------------------------------------------------------
//
//      Application's functions implementation
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Application's functions implementation
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#ifndef     APP_H
#define     APP_H

#include    <QApplication>
#include    <QCommandLineParser>
#include    <QMainWindow>

#include    "qtosgwidget.h"
#include    "tcp-client.h"

#include    "launcher-command-line.h"
#include    "launcher-config.h"
#include    "filesystem.h"
#include    "sim-process.h"

/*!
 * \class
 * \brief Main application
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LauncherApp : public QApplication
{
    Q_OBJECT

public:

    /// Constructor
    explicit LauncherApp(int argc, char **argv);
    /// Destructor
    virtual ~LauncherApp();

    int exec();

signals:

    /// Start simulation process
    void startSimulation();
    /// Stop simulation process
    void stopSimulation();

private:

    /// Simulation process controller
    SimProcess  *sim_process;

    /// Main window
    QMainWindow window;

    /// TCP-client to exchange data with simulator
    TcpClient   tcp_client;

    /// Launcher configuration
    launcher_config_t       launcher_config;
    /// Launcher command line
    launcher_command_line_t command_line;

    /// Command line parser
    QCommandLineParser  parser;

    /// Filesystem object
    FileSystem          fs;

    /// Parsing of command line
    CommandLineParesrResult parseCommandLine(QCommandLineParser &parser,
                                             launcher_command_line_t &command_line,
                                             QString &errorMessage);

    /// Process command line
    void commandLineProcess();

    /// Launcher initialization
    void init(launcher_config_t launcher_config);

    /// Override the default configuration by command line parameters
    void overrideByCommandLine(const launcher_command_line_t &command_line,
                               launcher_config_t &launcher_config);

    /// Read parameters from config file
    void loadConfig(QString cfg_path);

private slots:

    void debugPrint(QString msg);
};

#endif // APP_H
