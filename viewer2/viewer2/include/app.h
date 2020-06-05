#ifndef     APP_H
#define     APP_H

#include    <QApplication>
#include    <QCommandLineParser>

#include    "qthread-viewer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class App : public QApplication
{
public:

    App(int argc, char *argv[]);

    ~App();

    int exec();

private:

    QString route_dir;

    QString train_config;

    QCommandLineParser  parser;

    QThreadViewer       *viewer;

    enum CmdlineParseResult
    {
        CmdlineOk,
        CmdlineError,
        CmdlineVersion,
        CmdlineHelp
    };

    CmdlineParseResult parseCmdLine(QCommandLineParser &p, QString &errorMessage);

    bool init();
};

#endif // APP_H
