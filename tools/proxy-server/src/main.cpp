#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QString>

#include    <proxy-server.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("proxy-server");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Railway Simulator network proxy");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption configOption("config",
        "Configuration file path (XML format)", "config");
    parser.addOption(configOption);

    parser.process(app);

    // Путь к конфигу: параметр или значение по умолчанию
    QString configPath = parser.value(configOption);
    if (configPath.isEmpty())
    {
        configPath = "../cfg/proxy-server.xml";
    }

    ProxyServer proxy;

    if (!proxy.init(configPath))
    {
        qCritical() << "Failed to init proxy from:" << configPath;
        return 1;
    }

    qInfo().noquote() << "Proxy is running. Press Ctrl+C to stop.";

    return app.exec();
}
