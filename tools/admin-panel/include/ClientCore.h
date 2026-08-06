//------------------------------------------------------------------------------
//
//  Client core
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Client core
 *  \copyright SimulatorClient
 *  \date 2026
 */

#ifndef     CLIENTCORE_H
#define     CLIENTCORE_H

#include    <QObject>
#include    <QString>
#include    <QVector>
#include    <QMap>

#include    "ProtocolHandler.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct RouteData
{
    QString name;
    QString title;
    QString description;
    QString author;
    QString version;
    QString road;
    int scenariosCount;
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct ScenarioData
{
    QString name;           // Имя сценария
    QString dirName;        // Имя каталога (для запуска)
    QString description;    // Описание
    QString route;          // Маршрут
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class ClientCore : public QObject
{
    Q_OBJECT

public:

    explicit ClientCore(QObject* parent = nullptr);
    ~ClientCore();

    bool connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    void loadRoutes();
    void loadScenarios(const QString& route);
    void startSimulation(const QString& route, const QString& scenario);
    void stopSimulation();
    void getStatus();

    QVector<RouteData> getRoutes() const { return m_routes; }
    QVector<ScenarioData> getScenarios() const { return m_scenarios; }

signals:

    void connected();
    void disconnected();
    void error(const QString& error);

    void routesLoaded();
    void scenariosLoaded();
    void simulationStarted();
    void simulationStopped();
    void statusUpdated(bool isRunning, const QString& route, const QString& scenario);

private slots:

    void onConnected();
    void onDisconnected();
    void onError(const QString& error);
    void onRoutesReceived(const QJsonArray& routes);
    void onScenariosReceived(const QJsonArray& scenarios);
    void onStatusReceived(const QJsonObject& status);
    void onSimulationStarted();
    void onSimulationStopped();

private:

    ProtocolHandler*    m_protocol;
    QVector<RouteData>  m_routes;
    QVector<ScenarioData> m_scenarios;
    QString             m_currentRoute;
    bool                m_simulationRunning;
};

#endif // CLIENTCORE_H