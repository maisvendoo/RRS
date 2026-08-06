//------------------------------------------------------------------------------
//
//  Protocol handler for client
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#include    "ProtocolHandler.h"
#include    <QDebug>
#include    <QDateTime>
#include    <QThread>

//-----------------------------------------------------------------------------
ProtocolHandler::ProtocolHandler(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_port(0)
{
    connect(m_socket, &QTcpSocket::connected, this, &ProtocolHandler::connected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ProtocolHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ProtocolHandler::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [this](QAbstractSocket::SocketError socketError) {
        QString errorMsg;
        switch (socketError) {
            case QAbstractSocket::ConnectionRefusedError:
                errorMsg = "Connection refused";
                break;
            case QAbstractSocket::RemoteHostClosedError:
                errorMsg = "Remote host closed connection";
                break;
            case QAbstractSocket::HostNotFoundError:
                errorMsg = "Host not found";
                break;
            case QAbstractSocket::SocketTimeoutError:
                errorMsg = "Connection timeout";
                break;
            default:
                errorMsg = m_socket->errorString();
                break;
        }
        emit error(errorMsg);
        qWarning() << "Socket error:" << errorMsg;
    });

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(30000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &ProtocolHandler::onHeartbeat);
    m_heartbeatTimer->start();
}

//-----------------------------------------------------------------------------
ProtocolHandler::~ProtocolHandler()
{
    disconnectFromServer();
}

//-----------------------------------------------------------------------------
bool ProtocolHandler::connectToServer(const QString& host, quint16 port)
{
    if (isConnected())
    {
        disconnectFromServer();
    }

    m_host = host;
    m_port = port;

    qInfo() << "Connecting to" << host << ":" << port;
    m_socket->connectToHost(host, port);

    return m_socket->waitForConnected(5000);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::disconnectFromServer()
{
    if (m_socket)
    {
        m_socket->disconnectFromHost();
        m_heartbeatTimer->stop();
    }
}

//-----------------------------------------------------------------------------
bool ProtocolHandler::isConnected() const
{
    return m_socket && m_socket->state() == QTcpSocket::ConnectedState;
}

//-----------------------------------------------------------------------------
void ProtocolHandler::sendMessage(const QJsonObject& message)
{
    if (!isConnected())
    {
        return;
    }

    // Формируем JSON без лишних пробелов
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    
    // Отладочный вывод
    qDebug() << "Sending:" << data;
    
    // Отправляем
    qint64 written = m_socket->write(data);
    if (written != data.size())
    {
        qWarning() << "Failed to send complete message, sent:" << written << "of" << data.size();
    }
    m_socket->flush();
}

//-----------------------------------------------------------------------------
void ProtocolHandler::getRoutes()
{
    QJsonObject message;
    message["type"] = "GET_ROUTES";
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::getScenarios(const QString& route)
{
    QJsonObject message;
    message["type"] = "GET_SCENARIOS";
    message["route"] = route;
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::startSimulation(const QString& route, const QString& scenario)
{
    QJsonObject message;
    message["type"] = "START_SIMULATION";
    message["route"] = route;
    message["scenario"] = scenario;
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::stopSimulation()
{
    QJsonObject message;
    message["type"] = "STOP_SIMULATION";
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::getStatus()
{
    QJsonObject message;
    message["type"] = "GET_STATUS";
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    qDebug() << "Buffer size:" << m_buffer.size() << "Data:" << m_buffer;

    while (true)
    {
        int index = m_buffer.indexOf('\n');
        if (index == -1)
        {
            qDebug() << "No complete message, waiting for more data";
            break;
        }

        QByteArray messageData = m_buffer.left(index);
        m_buffer.remove(0, index + 1);

        qDebug() << "Extracted message:" << messageData;

        // Пропускаем пустые
        if (messageData.isEmpty())
        {
            continue;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(messageData, &parseError);
        
        if (parseError.error != QJsonParseError::NoError)
        {
            qWarning() << "JSON parse error:" << parseError.errorString() << "Data:" << messageData;
            continue;
        }

        if (!doc.isObject())
        {
            qWarning() << "Not a JSON object:" << messageData;
            continue;
        }

        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();

        qDebug() << "Received type:" << type;

        if (type == "HEARTBEAT")
        {
            continue;
        }
        else if (type == "ROUTE_LIST")
        {
            emit routesReceived(obj["routes"].toArray());
        }
        else if (type == "SCENARIO_LIST")
        {
            emit scenariosReceived(obj["scenarios"].toArray());
        }
        else if (type == "STATUS")
        {
            emit statusReceived(obj["status"].toObject());
        }
        else if (type == "SIMULATION_STARTED")
        {
            emit simulationStarted();
        }
        else if (type == "SIMULATION_STOPPED")
        {
            emit simulationStopped();
        }
        else if (type == "SUCCESS")
        {
            emit messageReceived(obj);
        }
        else if (type == "ERROR")
        {
            emit error(obj["message"].toString());
        }
        else
        {
            emit messageReceived(obj);
        }
    }
}

//-----------------------------------------------------------------------------
void ProtocolHandler::onDisconnected()
{
    m_heartbeatTimer->stop();
    emit disconnected();
    qInfo() << "Disconnected from server";
}

//-----------------------------------------------------------------------------
void ProtocolHandler::onHeartbeat()
{
    if (isConnected())
    {
        QJsonObject heartbeat;
        heartbeat["type"] = "HEARTBEAT";
        heartbeat["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        sendMessage(heartbeat);
    }
}