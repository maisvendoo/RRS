#ifndef     APP_H
#define     APP_H

#include    <QApplication>
#include    <QCommandLineParser>

#include    "command-line.h"
#include    "qviewer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class App : QApplication
{
public:

    App(int argc, char *argv[]);

    ~App();

    int exec();

private:

    QCommandLineParser  parser;

    command_line_t      cmd_line;

    QViewer             viewer;

    enum CmdlineParseResult
    {
        CmdlineOk,
        CmdlineError,
        CmdlineVersion,
        CmdlineHelp
    };

    CmdlineParseResult parseCmdLine(QCommandLineParser &p, QString &errorMessage);

    bool init();

    QString get_config_dir();

    bool load_settings(QString cfg_path);
};

#endif // APP_H
