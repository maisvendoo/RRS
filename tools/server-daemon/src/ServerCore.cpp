//------------------------------------------------------------------------------
//
//  Server core
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Server core
 *  \copyright SimulatorServer
 *  \date 2026
 */

#include    "ServerCore.h"
#include    <QDateTime>
#include    <QCoreApplication>
#include    <QTextStream>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ServerCore::ServerCore(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_initialized(false)
{
    Config& config = Config::instance();

    // Загрузка маршрутов
    if (!m_routeManager.loadRoutes(config.getRoutesPath()))
    {
        qWarning() << "No routes loaded";
    }

    // Подключение сигналов симулятора
    connect(&m_simulatorController, &SimulatorController::simulationError,
            this, &ServerCore::onSimulationError);

    connect(&m_simulatorController, &SimulatorController::simulationOutput,
            this, &ServerCore::onSimulationOutput);

    connect(&m_simulatorController, &SimulatorController::simulationStarted,
            this, &ServerCore::onSimulationStarted);

    connect(&m_simulatorController, &SimulatorController::simulationStopped,
            this, &ServerCore::onSimulationStopped);

    // Таймер для heartbeat
    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(30000);
    connect(m_heartbeatTimer, &QTimer::timeout,
            this, &ServerCore::onHeartbeat);

    // Таймер для статуса
    m_statusTimer = new QTimer(this);
    m_statusTimer->setInterval(60000);
    connect(m_statusTimer, &QTimer::timeout,
            this, &ServerCore::onHeartbeat);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ServerCore::~ServerCore()
{
    stop();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ServerCore::start()
{
    Config& config = Config::instance();

    if (!m_server->listen(QHostAddress::Any, config.getServerPort()))
    {
        qCritical() << "Failed to start server on port:" << config.getServerPort()
                    << "Error:" << m_server->errorString();
        return false;
    }

    connect(m_server, &QTcpServer::newConnection,
            this, &ServerCore::onNewConnection);

    m_heartbeatTimer->start();
    m_statusTimer->start();

    writePidFile();

    m_initialized = true;

    qInfo() << "Server started on port:" << config.getServerPort();
    qInfo() << "Routes loaded:" << m_routeManager.getRoutesCount();
    qInfo() << "Simulator path:" << config.getSimulatorPath();

    emit serverStarted();

    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::stop()
{
    if (!m_initialized)
    {
        return;
    }

    m_server->close();

    // Отключаем всех клиентов
    QMutexLocker locker(&m_clientsMutex);
    for (ProtocolHandler* client : m_clients.values())
    {
        client->deleteLater();
    }
    m_clients.clear();
    locker.unlock();

    // Останавливаем симуляцию
    m_simulatorController.stopSimulation();

    m_heartbeatTimer->stop();
    m_statusTimer->stop();

    // Удаляем PID файл
    QFile::remove(Config::instance().getPidFile());

    m_initialized = false;

    qInfo() << "Server stopped";

    emit serverStopped();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int ServerCore::getClientCount() const
{
    QMutexLocker locker(&m_clientsMutex);  // Теперь работает, т.к. m_clientsMutex объявлен как mutable
    return m_clients.size();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::onNewConnection()
{
    QTcpSocket* socket = m_server->nextPendingConnection();
    if (!socket)
    {
        return;
    }

    ProtocolHandler* handler = new ProtocolHandler(socket, this);

    connect(handler, &ProtocolHandler::messageReceived,
            this, [this, handler](const QJsonObject& msg)
    {
        onClientMessage(msg, handler);
    });

    connect(handler, &ProtocolHandler::clientDisconnected,
            this, [this, handler]()
    {
        onClientDisconnected(handler);
    });

    {
        QMutexLocker locker(&m_clientsMutex);
        m_clients[socket] = handler;
    }

    QString address = handler->getClientAddress();
    qInfo() << "Client connected:" << address;

    emit clientConnected(address);

    // Отправляем список маршрутов
    handleGetRoutes(handler);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::onClientMessage(const QJsonObject& message, ProtocolHandler* client)
{
    qDebug() << "Received message from" << client->getClientAddress() << ":" << message;
    processMessage(client, message);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::onClientDisconnected(ProtocolHandler* client)
{
    QString address = client->getClientAddress();

    {
        QMutexLocker locker(&m_clientsMutex);
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(client->parent());
        if (socket)
        {
            m_clients.remove(socket);
        }
    }

    qInfo() << "Client disconnected:" << address;

    emit clientDisconnected(address);

    client->deleteLater();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::onSimulationOutput(const QString& output)
{
    qDebug() << "Simulation output:" << output;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::onSimulationError(const QString& error)
{
    qWarning() << "Simulation error:" << error;

    // Отправляем ошибку всем клиентам
    QMutexLocker locker(&m_clientsMutex);
    for (ProtocolHandler* client : m_clients.values())
    {
        client->sendError("Simulation error: " + error);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::onSimulationStarted()
{
    qInfo() << "Simulation started:"
            << m_simulatorController.getCurrentRoute()
            << "/" << m_simulatorController.getCurrentScenario();

    emit simulationStarted(m_simulatorController.getCurrentRoute(),
                          m_simulatorController.getCurrentScenario());

    // Оповещаем всех клиентов
    QMutexLocker locker(&m_clientsMutex);
    QJsonObject notification;
    notification["type"] = "SIMULATION_STARTED";
    notification["route"] = m_simulatorController.getCurrentRoute();
    notification["scenario"] = m_simulatorController.getCurrentScenario();

    for (ProtocolHandler* client : m_clients.values())
    {
        client->sendMessage(notification);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::onSimulationStopped()
{
    qInfo() << "Simulation stopped";

    emit simulationStopped();

    // Оповещаем всех клиентов
    QMutexLocker locker(&m_clientsMutex);
    QJsonObject notification;
    notification["type"] = "SIMULATION_STOPPED";

    for (ProtocolHandler* client : m_clients.values())
    {
        client->sendMessage(notification);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::onHeartbeat()
{
    // Проверяем активность клиентов
    // Удаляем отключенных
    QMutexLocker locker(&m_clientsMutex);

    QList<QTcpSocket*> toRemove;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        if (!it.value()->isConnected())
        {
            toRemove.append(it.key());
        }
    }

    for (QTcpSocket* socket : toRemove)
    {
        ProtocolHandler* handler = m_clients.take(socket);
        handler->deleteLater();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::writePidFile()
{
    QString pidFile = Config::instance().getPidFile();
    QFile file(pidFile);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << QCoreApplication::applicationPid();
        file.close();

        qInfo() << "PID file written:" << pidFile;
    }
    else
    {
        qWarning() << "Failed to write PID file:" << pidFile;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::processMessage(ProtocolHandler* client, const QJsonObject& message)
{
    QString type = message["type"].toString();

    if (type == "GET_ROUTES")
    {
        handleGetRoutes(client);
    }
    else if (type == "GET_SCENARIOS")
    {
        QString route = message["route"].toString();
        handleGetScenarios(client, route);
    }
    else if (type == "START_SIMULATION")
    {
        QString route = message["route"].toString();
        QString scenario = message["scenario"].toString();
        handleStartSimulation(client, route, scenario);
    }
    else if (type == "STOP_SIMULATION")
    {
        handleStopSimulation(client);
    }
    else if (type == "GET_STATUS")
    {
        handleGetStatus(client);
    }
    else if (type == "GET_INFO")
    {
        handleGetInfo(client);
    }
    else if (type == "HEARTBEAT")
    {
        // Просто игнорируем
    }
    else
    {
        client->sendError("Unknown command type: " + type);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::handleGetRoutes(ProtocolHandler* client)
{
    QJsonArray routesArray;

    for (const RouteInfo& route : m_routeManager.getRoutes())
    {
        QJsonObject routeObj;
        routeObj["name"] = route.name;
        routeObj["title"] = route.title;
        routeObj["description"] = route.description;
        routeObj["author"] = route.author;
        routeObj["version"] = route.version;
        routeObj["road"] = route.road;

        // Количество сценариев
        ScenarioManager tempManager;
        if (tempManager.loadScenarios(route.routePath))
        {
            routeObj["scenarios_count"] = tempManager.getScenariosCount();
        }
        else
        {
            routeObj["scenarios_count"] = 0;
        }

        routesArray.append(routeObj);
    }

    client->sendRouteList(routesArray);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::handleGetScenarios(ProtocolHandler* client, const QString& route)
{
    RouteInfo routeInfo = m_routeManager.getRouteByName(route);
    if (routeInfo.name.isEmpty())
    {
        client->sendError("Route not found: " + route);
        return;
    }

    if (!m_scenarioManager.loadScenarios(routeInfo.routePath))
    {
        client->sendError("No scenarios found for route: " + route);
        return;
    }

    QJsonArray scenariosArray;
    for (const ScenarioInfo& scenario : m_scenarioManager.getScenarios())
    {
        QJsonObject scenarioObj;
        scenarioObj["name"] = scenario.name;
        scenarioObj["description"] = scenario.description;
        scenarioObj["type"] = scenario.type;
        scenarioObj["route"] = scenario.route;
        scenariosArray.append(scenarioObj);
    }

    client->sendScenarioList(scenariosArray);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::handleStartSimulation(ProtocolHandler* client,
                                       const QString& route,
                                       const QString& scenario)
{
    // Проверяем существование маршрута
    RouteInfo routeInfo = m_routeManager.getRouteByName(route);
    if (routeInfo.name.isEmpty())
    {
        client->sendError("Route not found: " + route);
        return;
    }

    // Проверяем существование сценария
    if (!m_scenarioManager.loadScenarios(routeInfo.routePath))
    {
        client->sendError("Could not load scenarios for route: " + route);
        return;
    }

    ScenarioInfo scenarioInfo = m_scenarioManager.getScenarioByName(scenario);
    if (scenarioInfo.name.isEmpty())
    {
        client->sendError("Scenario not found: " + scenario);
        return;
    }

    // Проверяем, не запущена ли уже симуляция
    if (m_simulatorController.isRunning())
    {
        client->sendError("Simulation is already running. Stop it first.");
        return;
    }

    // Запускаем симуляцию
    Config& config = Config::instance();
    bool success = m_simulatorController.startSimulation(
        route, scenario, config.getSimulatorPath()
    );

    if (success)
    {
        client->sendSuccess("Simulation started successfully");
        qInfo() << "Simulation started by" << client->getClientAddress()
                << "route:" << route << "scenario:" << scenario;
    }
    else
    {
        client->sendError("Failed to start simulation");
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::handleStopSimulation(ProtocolHandler* client)
{
    if (!m_simulatorController.isRunning())
    {
        client->sendError("No simulation running");
        return;
    }

    bool success = m_simulatorController.stopSimulation();
    if (success)
    {
        client->sendSuccess("Simulation stopped successfully");
        qInfo() << "Simulation stopped by" << client->getClientAddress();
    }
    else
    {
        client->sendError("Failed to stop simulation");
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::handleGetStatus(ProtocolHandler* client)
{
    QJsonObject status;
    status["running"] = m_simulatorController.isRunning();

    if (m_simulatorController.isRunning())
    {
        status["route"] = m_simulatorController.getCurrentRoute();
        status["scenario"] = m_simulatorController.getCurrentScenario();
        status["uptime_seconds"] = m_simulatorController.getUptimeSeconds();
    }

    status["clients_connected"] = getClientCount();
    status["server_uptime"] = m_initialized ?
        QDateTime::currentMSecsSinceEpoch() / 1000 : 0;

    client->sendStatus(status);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ServerCore::handleGetInfo(ProtocolHandler* client)
{
    QJsonObject info;
    info["server_name"] = "Railway Simulator Server";
    info["version"] = "1.0.0";
    info["port"] = getPort();
    info["routes_count"] = m_routeManager.getRoutesCount();
    info["protocol_version"] = "1.0";

    client->sendMessage(info);
}