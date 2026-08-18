//------------------------------------------------------------------------------
//
//  Server core
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------

#ifndef     SERVERCORE_H
#define     SERVERCORE_H

#include    <QObject>
#include    <QTcpServer>
#include    <QTcpSocket>
#include    <QMap>
#include    <QMutex>
#include    <QTimer>
#include    <QFile>
#include    <QHostAddress>
#include    <QJsonObject>
#include    <QJsonArray>
#include    <QJsonValue>

#include    "RouteManager.h"
#include    "ScenarioManager.h"
#include    "SimulatorController.h"
#include    "ProtocolHandler.h"
#include    "Config.h"

//-----------------------------------------------------------------------------
class ServerCore : public QObject
{
    Q_OBJECT

public:

    explicit ServerCore(QObject* parent = nullptr);
    ~ServerCore();

    bool start();
    void stop();
    bool isRunning() const { return m_server && m_server->isListening(); }

    int getClientCount() const;
    quint16 getPort() const { return Config::instance().getServerPort(); }

signals:

    void serverStarted();
    void serverStopped();
    void clientConnected(const QString& address);
    void clientDisconnected(const QString& address);
    void simulationStarted(const QString& route, const QString& scenario);
    void simulationStopped();

private slots:

    void onNewConnection();
    void onClientMessage(const QJsonObject& message, ProtocolHandler* client);
    void onClientDisconnected(ProtocolHandler* client);
    void onSimulationOutput(const QString& output);
    void onSimulationError(const QString& error);
    void onSimulationStarted();
    void onSimulationStopped();
    void onHeartbeat();
    void writePidFile();

private:

    void processMessage(ProtocolHandler* client, const QJsonObject& message);
    void handleAuth(ProtocolHandler* client, const QJsonObject& message);
    void handleGetRoutes(ProtocolHandler* client);
    void handleGetScenarios(ProtocolHandler* client, const QString& route);
    void handleStartSimulation(ProtocolHandler* client, const QString& route, const QString& scenario);
    void handleStopSimulation(ProtocolHandler* client);
    void handleGetStatus(ProtocolHandler* client);
    void handleGetInfo(ProtocolHandler* client);
    void sendCurrentStatus(ProtocolHandler* client);     // <-- ДОБАВИТЬ

    QTcpServer*                         m_server;
    RouteManager                        m_routeManager;
    ScenarioManager                     m_scenarioManager;
    SimulatorController                 m_simulatorController;

    QMap<QTcpSocket*, ProtocolHandler*> m_clients;
    mutable QMutex                      m_clientsMutex;

    QTimer*                             m_heartbeatTimer;
    QTimer*                             m_statusTimer;

    bool                                m_initialized;
};

#endif // SERVERCORE_H