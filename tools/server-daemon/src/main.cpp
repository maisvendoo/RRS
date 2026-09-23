//------------------------------------------------------------------------------
//
//  Simulator server main
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Simulator server main
 *  \copyright SimulatorServer
 *  \date 2026
 */

#include    <QCoreApplication>
#include    <QCommandLineParser>
#include    <QDebug>
#include    <QFile>
#include    <QTextStream>
#include    <QTimer>
#include    <csignal>

#include    "ServerCore.h"
#include    "Config.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static ServerCore* g_server = nullptr;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void signalHandler(int signal)
{
    qInfo() << "Received signal" << signal << ", shutting down...";
    
    if (g_server)
    {
        g_server->stop();
    }
    
    QCoreApplication::quit();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("SimulatorServer");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Railway Simulator Server");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption configOption("config",
        "Configuration file path (XML format)", "config");
    parser.addOption(configOption);

    QCommandLineOption portOption("port",
        "Server port (overrides config)", "port");
    parser.addOption(portOption);

    QCommandLineOption daemonOption("daemon",
        "Run as daemon");
    parser.addOption(daemonOption);

    parser.process(app);

    // Загрузка конфигурации
    QString configPath = parser.value(configOption);
    if (configPath.isEmpty())
    {
        configPath = "../cfg/daemon-config.xml";
    }

    if (!Config::instance().load(configPath))
    {
        qCritical() << "Failed to load configuration from:" << configPath;
        return 1;
    }

    // Переопределение порта из командной строки
    if (parser.isSet(portOption))
    {
        bool ok;
        int port = parser.value(portOption).toInt(&ok);
        if (ok && port > 0 && port < 65536)
        {
            Config::instance().setServerPort(static_cast<quint16>(port));
            qInfo() << "Port overridden to:" << port;
        }
        else
        {
            qWarning() << "Invalid port specified, using config value";
        }
    }

    // Запуск в фоновом режиме (daemon)
    if (parser.isSet(daemonOption))
    {
        qInfo() << "Running in daemon mode";
    }

    // Создание и запуск сервера
    ServerCore server;
    g_server = &server;

    if (!server.start())
    {
        qCritical() << "Failed to start server";
        return 1;
    }

    qInfo() << "Server is running. Press Ctrl+C to stop.";

    // Установка обработчиков сигналов
    ::signal(SIGINT, signalHandler);
    ::signal(SIGTERM, signalHandler);

    // Обработка завершения приложения
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&server]()
    {
        server.stop();
        g_server = nullptr;
    });

    return app.exec();
}
