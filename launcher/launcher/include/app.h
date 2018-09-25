//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     APP_H
#define     APP_H

#include    <QApplication>
#include    <QCommandLineParser>

#include    "launcher-command-line.h"
#include    "launcher-config.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LauncherApp : public QApplication
{
    Q_OBJECT

public:

    explicit LauncherApp(int argc, char **argv);
    virtual ~LauncherApp();

    launcher_config_t getConfig() const;

private:

    launcher_config_t       launcher_config;
    launcher_command_line_t command_line;

    QCommandLineParser  parser;

    CommandLineParesrResult parseCommandLine(QCommandLineParser &parser,
                                             launcher_command_line_t &command_line,
                                             QString &errorMessage);

    void init();

    void overrideByCommandLine(const launcher_command_line_t &command_line,
                               launcher_config_t &launcher_config);
};

#endif // APP_H
