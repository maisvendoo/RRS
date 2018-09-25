#include    "app.h"

#include    "convert.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LauncherApp::LauncherApp(int argc, char **argv) : QApplication(argc, argv)
{
    init();
    overrideByCommandLine(command_line, launcher_config);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LauncherApp::~LauncherApp()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
launcher_config_t LauncherApp::getConfig() const
{
    return launcher_config;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CommandLineParesrResult LauncherApp::parseCommandLine(QCommandLineParser &parser,
                                                      launcher_command_line_t &command_line,
                                                      QString &errorMessage)
{
    QCommandLineOption help = parser.addHelpOption();
    QCommandLineOption version = parser.addVersionOption();

    // Width of graphic's window
    QCommandLineOption width(QStringList() << "w" << "width",
                             QApplication::translate("main", "Window width"),
                             QApplication::translate("main", "window-width"));

    parser.addOption(width);

    // Height of graphic's window
    QCommandLineOption height(QStringList() << "s" << "height",
                              QApplication::translate("main", "Window height"),
                              QApplication::translate("main", "window-height"));

    parser.addOption(height);

    // Fullscreen
    QCommandLineOption fullscreen(QStringList() << "f" << "fullscreen",
                                  QApplication::translate("main", "Fullscreen mode"));

    parser.addOption(fullscreen);

    // Host address
    QCommandLineOption hostAddress(QStringList() << "a" << "host",
                                   QApplication::translate("main", "Host address"),
                                   QApplication::translate("main", "host-ip-address"));

    parser.addOption(hostAddress);

    // Host port
    QCommandLineOption port(QStringList() << "p" << "port",
                            QApplication::translate("main", "Host port"),
                            QApplication::translate("main", "port-number"));

    parser.addOption(port);

    // Start as remote client
    QCommandLineOption remoteClient(QStringList() << "r" << "remote-client",
                                    QApplication::translate("main", "Start as remote client"));

    parser.addOption(remoteClient);

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

    if (parser.isSet(width))
    {
        command_line.width.is_present = TextToInt(parser.value(width),
                                                  command_line.width.value);
    }

    if (parser.isSet(height))
    {
        command_line.height.is_present = TextToInt(parser.value(height),
                                                   command_line.height.value);
    }


    command_line.fullscreen.is_present = command_line.fullscreen.value = parser.isSet(fullscreen);

    if (parser.isSet(hostAddress))
    {
        command_line.host_address.is_present = true;
        command_line.host_address.value = parser.value(hostAddress);
    }

    if (parser.isSet(port))
    {
        command_line.port.is_present = TextToInt(parser.value(port),
                                                 command_line.port.value);
    }

    command_line.remote_client.is_present = command_line.remote_client.value = parser.isSet(remoteClient);

    return CommandLineOk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LauncherApp::init()
{
    QString errorMessage = "";

    switch (parseCommandLine(parser, command_line, errorMessage))
    {
    case CommandLineOk:
        return;

    case CommandLineError:
        fputs(qPrintable(errorMessage), stderr);
        fputs("\n", stderr);
        return;

    case CommandLineVersionRequired:
        printf("%s %s\n",
               qPrintable(this->applicationName()),
               qPrintable(this->applicationVersion()));

        quit();
        break;

    case CommandLineHelpRequired:

        parser.showHelp();
        quit();
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LauncherApp::overrideByCommandLine(const launcher_command_line_t &command_line,
                                        launcher_config_t &launcher_config)
{
    if (command_line.width.is_present)
        launcher_config.width = command_line.width.value;

    if (command_line.height.is_present)
        launcher_config.height = command_line.height.value;

    if (command_line.fullscreen.is_present)
        launcher_config.fullscreen = command_line.fullscreen.value;

    if (command_line.host_address.is_present)
        launcher_config.host_address = command_line.host_address.value;

    if (command_line.port.is_present)
        launcher_config.port = command_line.port.value;

    if (command_line.remote_client.is_present)
        launcher_config.remote_client = command_line.remote_client.value;
}
