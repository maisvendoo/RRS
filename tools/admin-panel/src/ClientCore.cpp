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

#include    "ClientCore.h"
#include    <QDebug>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ClientCore::ClientCore(QObject* parent)
    : QObject(parent)
    , m_protocol(new ProtocolHandler(this))
    , m_simulationRunning(false)
{
    connect(m_protocol, &ProtocolHandler::connected,
            this, &ClientCore::onConnected);
    connect(m_protocol, &ProtocolHandler::disconnected,
            this, &ClientCore::onDisconnected);
    connect(m_protocol, &ProtocolHandler::error,
            this, &ClientCore::onError);

    connect(m_protocol, &ProtocolHandler::routesReceived,
            this, &ClientCore::onRoutesReceived);
    connect(m_protocol, &ProtocolHandler::scenariosReceived,
            this, &ClientCore::onScenariosReceived);
    connect(m_protocol, &ProtocolHandler::statusReceived,
            this, &ClientCore::onStatusReceived);
    connect(m_protocol, &ProtocolHandler::simulationStarted,
            this, &ClientCore::onSimulationStarted);
    connect(m_protocol, &ProtocolHandler::simulationStopped,
            this, &ClientCore::onSimulationStopped);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ClientCore::~ClientCore()
{
    disconnectFromServer();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ClientCore::connectToServer(const QString& host, quint16 port)
{
    return m_protocol->connectToServer(host, port);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::disconnectFromServer()
{
    m_protocol->disconnectFromServer();
    m_simulationRunning = false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ClientCore::isConnected() const
{
    return m_protocol->isConnected();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::loadRoutes()
{
    if (isConnected())
    {
        m_protocol->getRoutes();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::loadScenarios(const QString& route)
{
    if (isConnected())
    {
        m_currentRoute = route;
        m_protocol->getScenarios(route);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::startSimulation(const QString& route, const QString& scenario)
{
    if (isConnected())
    {
        m_protocol->startSimulation(route, scenario);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::stopSimulation()
{
    if (isConnected())
    {
        m_protocol->stopSimulation();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::getStatus()
{
    if (isConnected())
    {
        m_protocol->getStatus();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::onConnected()
{
    qInfo() << "Connected to server";
    emit connected();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::onDisconnected()
{
    m_simulationRunning = false;
    qInfo() << "Disconnected from server";
    emit disconnected();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::onError(const QString& errorMsg)
{
    qWarning() << "Error:" << errorMsg;
    emit error(errorMsg);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::onRoutesReceived(const QJsonArray& routes)
{
    m_routes.clear();

    for (const QJsonValue& value : routes)
    {
        QJsonObject obj = value.toObject();
        RouteData route;
        route.name = obj["name"].toString();
        route.title = obj["title"].toString();
        route.description = obj["description"].toString();
        route.author = obj["author"].toString();
        route.version = obj["version"].toString();
        route.road = obj["road"].toString();
        route.scenariosCount = obj["scenarios_count"].toInt();
        m_routes.append(route);
    }

    qInfo() << "Routes loaded:" << m_routes.size();
    emit routesLoaded();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::onScenariosReceived(const QJsonArray& scenarios)
{
    m_scenarios.clear();

    for (const QJsonValue& value : scenarios)
    {
        QJsonObject obj = value.toObject();
        ScenarioData scenario;
        scenario.name = obj["name"].toString();
        scenario.dirName = obj["dirName"].toString();
        scenario.description = obj["description"].toString();
        scenario.route = obj["route"].toString();
        m_scenarios.append(scenario);
    }

    qInfo() << "Scenarios loaded:" << m_scenarios.size();
    emit scenariosLoaded();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::onStatusReceived(const QJsonObject& status)
{
    bool running = status["running"].toBool();
    QString route = status["route"].toString();
    QString scenario = status["scenario"].toString();

    m_simulationRunning = running;

    emit statusUpdated(running, route, scenario);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::onSimulationStarted()
{
    m_simulationRunning = true;
    emit simulationStarted();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ClientCore::onSimulationStopped()
{
    m_simulationRunning = false;
    emit simulationStopped();
}
