//------------------------------------------------------------------------------
//
//  Server core
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------

#include    "ServerCore.h"
#include    <QDateTime>
#include    <QCoreApplication>
#include    <QTextStream>

//-----------------------------------------------------------------------------
ServerCore::ServerCore(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_initialized(false)
{
    Config& config = Config::instance();

    if (!m_routeManager.loadRoutes(config.getRoutesPath()))
    {
        qWarning() << "No routes loaded";
    }

    connect(&m_simulatorController, &SimulatorController::simulationError,
            this, &ServerCore::onSimulationError);

    connect(&m_simulatorController, &SimulatorController::simulationOutput,
            this, &ServerCore::onSimulationOutput);

    connect(&m_simulatorController, &SimulatorController::simulationStarted,
            this, &ServerCore::onSimulationStarted);

    connect(&m_simulatorController, &SimulatorController::simulationStopped,
            this, &ServerCore::onSimulationStopped);

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(30000);
    connect(m_heartbeatTimer, &QTimer::timeout,
            this, &ServerCore::onHeartbeat);

    m_statusTimer = new QTimer(this);
    m_statusTimer->setInterval(60000);
    connect(m_statusTimer, &QTimer::timeout,
            this, &ServerCore::onHeartbeat);
}

//-----------------------------------------------------------------------------
ServerCore::~ServerCore()
{
    stop();
}

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
void ServerCore::stop()
{
    if (!m_initialized)
    {
        return;
    }

    qInfo() << "Stopping server...";

    // Закрываем сервер для новых подключений
    m_server->close();

    // Отключаем всех клиентов
    {
        QMutexLocker locker(&m_clientsMutex);
        for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
        {
            ProtocolHandler* handler = it.value();
            if (handler)
            {
                handler->disconnect();
                handler->deleteLater();
            }
        }
        m_clients.clear();
    }

    // Останавливаем симуляцию
    m_simulatorController.stopSimulation();

    // Останавливаем таймеры
    m_heartbeatTimer->stop();
    m_statusTimer->stop();

    // Удаляем PID файл
    QFile::remove(Config::instance().getPidFile());

    m_initialized = false;

    qInfo() << "Server stopped";
    emit serverStopped();
}

//-----------------------------------------------------------------------------
int ServerCore::getClientCount() const
{
    QMutexLocker locker(&m_clientsMutex);
    return m_clients.size();
}

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

    // Отправляем текущий статус
    QTimer::singleShot(100, this, [this, handler]() {
        sendCurrentStatus(handler);
    });
}

//-----------------------------------------------------------------------------
void ServerCore::onClientMessage(const QJsonObject& message, ProtocolHandler* client)
{
    qDebug() << "Received message from" << client->getClientAddress() << ":" << message;
    processMessage(client, message);
}

//-----------------------------------------------------------------------------
void ServerCore::onClientDisconnected(ProtocolHandler* client)
{
    if (!client)
    {
        return;
    }

    QString address = client->getClientAddress();
    qInfo() << "Client disconnected:" << address;

    // Удаляем клиента из списка
    {
        QMutexLocker locker(&m_clientsMutex);
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(client->parent());
        if (socket)
        {
            m_clients.remove(socket);
        }
    }

    emit clientDisconnected(address);

    // client удалится автоматически через deleteLater в ProtocolHandler
}

//-----------------------------------------------------------------------------
void ServerCore::sendCurrentStatus(ProtocolHandler* client)          // <-- ДОБАВИТЬ
{
    if (!client || !client->isConnected())
    {
        return;
    }

    QJsonObject status;
    status["running"] = m_simulatorController.isRunning();

    if (m_simulatorController.isRunning())
    {
        status["route"] = m_simulatorController.getCurrentRoute();
        status["scenario"] = m_simulatorController.getCurrentScenario();
        status["uptime_seconds"] = m_simulatorController.getUptimeSeconds();
    }
    else
    {
        status["route"] = "";
        status["scenario"] = "";
        status["uptime_seconds"] = 0;
    }

    QJsonObject message;
    message["type"] = "STATUS";
    message["status"] = status;

    client->sendMessage(message);

    qDebug() << "Sent current status to client: running=" << m_simulatorController.isRunning();
}

//-----------------------------------------------------------------------------
void ServerCore::onSimulationOutput(const QString& output)
{
    qDebug() << "Simulation output:" << output;
}

//-----------------------------------------------------------------------------
void ServerCore::onSimulationError(const QString& error)
{
    qWarning() << "Simulation error:" << error;

    QMutexLocker locker(&m_clientsMutex);
    for (ProtocolHandler* client : m_clients.values())
    {
        client->sendError("Simulation error: " + error);
    }
}

//-----------------------------------------------------------------------------
void ServerCore::onSimulationStarted()
{
    qInfo() << "Simulation started:"
            << m_simulatorController.getCurrentRoute()
            << "/" << m_simulatorController.getCurrentScenario();

    emit simulationStarted(m_simulatorController.getCurrentRoute(),
                          m_simulatorController.getCurrentScenario());

    QMutexLocker locker(&m_clientsMutex);

    // Отправляем статус
    QJsonObject status;
    status["running"] = true;
    status["route"] = m_simulatorController.getCurrentRoute();
    status["scenario"] = m_simulatorController.getCurrentScenario();
    status["uptime_seconds"] = 0;

    QJsonObject message;
    message["type"] = "STATUS";
    message["status"] = status;

    for (ProtocolHandler* client : m_clients.values())
    {
        client->sendMessage(message);
    }

    // Отправляем уведомление
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
void ServerCore::onSimulationStopped()
{
    qInfo() << "Simulation stopped";

    emit simulationStopped();

    QMutexLocker locker(&m_clientsMutex);

    // Отправляем статус
    QJsonObject status;
    status["running"] = false;
    status["route"] = "";
    status["scenario"] = "";
    status["uptime_seconds"] = 0;

    QJsonObject message;
    message["type"] = "STATUS";
    message["status"] = status;

    for (ProtocolHandler* client : m_clients.values())
    {
        client->sendMessage(message);
    }

    // Отправляем уведомление
    QJsonObject notification;
    notification["type"] = "SIMULATION_STOPPED";

    for (ProtocolHandler* client : m_clients.values())
    {
        client->sendMessage(notification);
    }
}

//-----------------------------------------------------------------------------
void ServerCore::onHeartbeat()
{
    QMutexLocker locker(&m_clientsMutex);

    QList<QTcpSocket*> toRemove;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        ProtocolHandler* handler = it.value();
        if (!handler || !handler->isConnected())
        {
            toRemove.append(it.key());
        }
    }

    for (QTcpSocket* socket : toRemove)
    {
        ProtocolHandler* handler = m_clients.take(socket);
        if (handler)
        {
            qInfo() << "Removing dead client:" << handler->getClientAddress();
            handler->disconnect();
            handler->deleteLater();
        }
    }
}

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
        // Игнорируем
    }
    else
    {
        client->sendError("Unknown command type: " + type);
    }
}

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
        scenarioObj["dirName"] = scenario.dirName;
        scenarioObj["description"] = scenario.description;
        scenarioObj["route"] = scenario.route;
        scenariosArray.append(scenarioObj);
    }

    client->sendScenarioList(scenariosArray);
}

//-----------------------------------------------------------------------------
void ServerCore::handleStartSimulation(ProtocolHandler* client,
                                       const QString& route,
                                       const QString& scenario)
{
    qDebug() << "=== SERVER: Received START_SIMULATION ===";
    qDebug() << "  route:" << route;
    qDebug() << "  scenario:" << scenario;

    RouteInfo routeInfo = m_routeManager.getRouteByName(route);
    if (routeInfo.name.isEmpty())
    {
        client->sendError("Route not found: " + route);
        return;
    }

    if (!m_scenarioManager.loadScenarios(routeInfo.routePath))
    {
        client->sendError("Could not load scenarios for route: " + route);
        return;
    }

    ScenarioInfo scenarioInfo = m_scenarioManager.getScenarioByName(scenario);
    if (scenarioInfo.dirName.isEmpty())
    {
        client->sendError("Scenario not found: " + scenario);
        return;
    }

    if (m_simulatorController.isRunning())
    {
        client->sendError("Simulation is already running. Stop it first.");
        return;
    }

    Config& config = Config::instance();

    qInfo() << "Starting simulation: route=" << routeInfo.name << " scenario=" << scenarioInfo.dirName;

    bool success = m_simulatorController.startSimulation(
        routeInfo.name,
        scenarioInfo.dirName,
        config.getSimulatorPath()
    );

    if (success)
    {
        client->sendSuccess("Simulation started successfully");
        qInfo() << "Simulation started by" << client->getClientAddress()
                << "route:" << routeInfo.name << "scenario:" << scenarioInfo.dirName;
    }
    else
    {
        client->sendError("Failed to start simulation");
    }
}

//-----------------------------------------------------------------------------
void ServerCore::handleStopSimulation(ProtocolHandler* client)
{
    if (!m_simulatorController.isRunning())
    {
        client->sendError("No simulation running");
        return;
    }

    qInfo() << "Stopping simulation by client request";

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
    else
    {
        status["route"] = "";
        status["scenario"] = "";
        status["uptime_seconds"] = 0;
    }

    status["clients_connected"] = getClientCount();
    status["server_uptime"] = m_initialized ?
        QDateTime::currentMSecsSinceEpoch() / 1000 : 0;

    client->sendStatus(status);
}

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
