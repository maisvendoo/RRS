#include    <QApplication>
#include    <QCommandLineParser>
#include    "mainwindow.h"

QString get_module_path(QStringList args)
{
    QCommandLineParser parser;
    QCommandLineOption help = parser.addHelpOption();

    QCommandLineOption modulePath(QStringList() << "m" << "module-path",
                                  QApplication::translate("main", "Display module path"),
                                  QApplication::translate("main", "module-path"));

    parser.addOption(modulePath);

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
        return parser.value(modulePath);
    }

    QApplication::exit(0);

    return "";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w(get_module_path(app.arguments()));

    w.show();

    return app.exec();
}
