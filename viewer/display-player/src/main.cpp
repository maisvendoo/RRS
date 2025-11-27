#include    <QApplication>
#include    <QCommandLineParser>
#include    "mainwindow.h"

void parser(QStringList args, QString& module_path, QString& config_path)
{
    QCommandLineParser parser;
    QCommandLineOption help = parser.addHelpOption();

    QCommandLineOption modulePath(QStringList() << "m" << "module-path",
                                  QApplication::translate("main", "Display module path"),
                                  QApplication::translate("main", "module-path"));

    QCommandLineOption configPath(QStringList() << "c" << "config-path",
                                   QApplication::translate("main", "Path to folder with display config files"),
                                   QApplication::translate("main", "signals-path"));

    parser.addOption(modulePath);
    parser.addOption(configPath);

    if (!parser.parse(args))
    {
        QApplication::exit(0);
    }

    if (parser.isSet(help))
    {
        parser.showHelp();
    }

    if (parser.isSet(modulePath))
    {
        module_path = parser.value(modulePath);

        if (parser.isSet(configPath))
            config_path = parser.value(configPath);

        return;
    }

    QApplication::exit(0);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //app.setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);

    QString module_path = "";
    QString config_path = "";
    parser(app.arguments(), module_path, config_path);
    MainWindow w(module_path, config_path);

    w.show();

    return app.exec();
}
