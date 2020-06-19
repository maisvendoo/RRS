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

    case CmdlineVersion:

        parser.showVersion();
    }

    return viewer.run();
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
        cmd_line.train_config = p.value(train_cfg);
    }

    if (p.isSet(route))
    {
        cmd_line.route_dir = p.value(route);
    }

    return CmdlineOk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool App::init()
{
    load_settings(get_config_dir() + QDir::separator() + "settings.xml");

    viewer.setUpViewInWindow(0, 0, 1280, 720);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString App::get_config_dir()
{
    QDir work_dir(this->applicationDirPath());
    work_dir.cdUp();

    QString cfg_path = QDir::toNativeSeparators(work_dir.path()) + QDir::separator() + "cfg";

    return cfg_path;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool App::load_settings(QString cfg_path)
{
    return true;
}
