#include    <mainwindow.h>

#include    <QApplication>
#include    <QLocale>
#include    <QTranslator>
#include    <QCommandLineParser>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    QCommandLineParser parser;
    route_map_command_line_t cmd_line;

    QCommandLineOption optHelp = parser.addHelpOption();
    QCommandLineOption optHostAddress(QStringList() << "host" << "host-address",
                                      QCoreApplication::translate("main", "Host address, default: 127.0.0.1"),
                                      QCoreApplication::translate("main", "host address"));
    QCommandLineOption optPort(QStringList() << "p" << "port",
                               QCoreApplication::translate("main", "Port, default: 1992"),
                               QCoreApplication::translate("main", "port"));

    parser.addOption(optHostAddress);
    parser.addOption(optPort);
    parser.parse(a.arguments());

    if (parser.isSet(optHelp))
    {
        parser.showHelp();
        return -1;
    }

    if (parser.isSet(optHostAddress))
    {
        cmd_line.host_addr.is_present = true;
        cmd_line.host_addr.value = parser.value(optHostAddress);
    }

    if (parser.isSet(optPort))
    {
        int port = parser.value(optPort).toInt();
        if (port >= 0 && port <= 65535)
        {
            cmd_line.port.is_present = true;
            cmd_line.port.value = static_cast<quint16>(port);
        }
    }

    QTranslator translator;

    if (translator.load("route-map.ru_RU.qm", ":/translations/translations"))
    {
        a.installTranslator(&translator);
    }

    MainWindow w(cmd_line);
    w.show();
    return a.exec();
}
