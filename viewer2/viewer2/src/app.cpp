#include    "app.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
App::App(int argc, char *argv[]) : QApplication(argc, argv)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
App::~App()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int App::exec()
{
    QString errorMsg = "";

    switch (parseCmdLine(parser, errorMsg))
    {
    case CmdlineOk:

        if (!init())
        {
            return -1;
        }

        break;

    case CmdlineError:

        fputs(qPrintable(errorMsg), stderr);
        fputs("\n", stderr);
        return -1;

    case CmdlineHelp:

        parser.showHelp();
        return 0;

    case CmdlineVersion:

        return 0;
    }

    return QApplication::exec();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
App::CmdlineParseResult App::parseCmdLine(QCommandLineParser &p,
                                          QString &errorMessage)
{
    QCommandLineOption help = p.addHelpOption();
    QCommandLineOption version = p.addVersionOption();

    QCommandLineOption train_cfg(QStringList() << "t" << "train",
                                 QApplication::translate("main", "Train configuration file"),
                                 QApplication::translate("main", "train-config-file"));

    p.addOption(train_cfg);

    QCommandLineOption route(QStringList() << "r" << "route",
                             QApplication::translate("main", "Route directory"),
                             QApplication::translate("main", "route-directory"));

    p.addOption(route);

    if (!p.parse(this->arguments()))
    {
        errorMessage = p.errorText();
        return CmdlineError;
    }

    if (p.isSet(help))
    {
        return CmdlineHelp;
    }

    if (p.isSet(version))
    {
        return CmdlineVersion;
    }

    if (p.isSet(train_cfg))
    {
        train_config = p.value(train_cfg);
    }

    if (p.isSet(route))
    {
        route_dir = p.value(route);
    }

    return CmdlineOk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool App::init()
{
    QString work_dir = QApplication::applicationDirPath();
    QDir root_dir(work_dir);
    root_dir.cdUp();

    QString route_path = root_dir.path() +
            QDir::separator() +
            "routes" +
            QDir::separator() +
            route_dir;

    QString train_path = root_dir.path() +
            QDir::separator() +
            "cfg" +
            QDir::separator() +
            "trains" +
            QDir::separator() +
            train_config + ".xml";

    viewer = new QThreadViewer(root_dir.path(), route_path, train_path);
    viewer->start();

    return true;
}
