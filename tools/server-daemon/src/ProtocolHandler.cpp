//------------------------------------------------------------------------------
//
//  Protocol handler
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Protocol handler
 *  \copyright SimulatorServer
 *  \date 2026
 */

#include    "ProtocolHandler.h"
#include    <QDebug>
#include    <QDateTime>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ProtocolHandler::ProtocolHandler(QTcpSocket* socket, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_lastActivity(QDateTime::currentMSecsSinceEpoch())
{
    connect(m_socket, &QTcpSocket::readyRead,
            this, &ProtocolHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &ProtocolHandler::onDisconnected);

    // Heartbeat timer
    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(30000); // 30 seconds
    connect(m_heartbeatTimer, &QTimer::timeout,
            this, &ProtocolHandler::onHeartbeatTimeout);
    m_heartbeatTimer->start();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ProtocolHandler::~ProtocolHandler()
{
    if (m_socket)
    {
        m_socket->close();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::sendMessage(const QJsonObject& message)
{
    if (!isConnected())
    {
        return;
    }

    QByteArray data = QJsonDocument(message).toJson();
    data.append('\n');
    m_socket->write(data);
    m_socket->flush();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::sendRouteList(const QJsonArray& routes)
{
    QJsonObject message;
    message["type"] = "ROUTE_LIST";
    message["routes"] = routes;
    message["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    sendMessage(message);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::sendScenarioList(const QJsonArray& scenarios)
{
    QJsonObject message;
    message["type"] = "SCENARIO_LIST";
    message["scenarios"] = scenarios;
    message["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    sendMessage(message);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::sendError(const QString& error)
{
    QJsonObject message;
    message["type"] = "ERROR";
    message["message"] = error;
    message["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    sendMessage(message);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::sendSuccess(const QString& message)
{
    QJsonObject response;
    response["type"] = "SUCCESS";
    response["message"] = message;
    response["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    sendMessage(response);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::sendStatus(const QJsonObject& status)
{
    QJsonObject message;
    message["type"] = "STATUS";
    message["status"] = status;
    message["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    sendMessage(message);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
QString ProtocolHandler::getClientAddress() const
{
    if (m_socket)
    {
        return m_socket->peerAddress().toString() + ":" +
               QString::number(m_socket->peerPort());
    }
    return QString();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::onReadyRead()
{
    m_lastActivity = QDateTime::currentMSecsSinceEpoch();
    m_buffer.append(m_socket->readAll());

    while (true)
    {
        int index = m_buffer.indexOf('\n');
        if (index == -1)
        {
            break;
        }

        QByteArray messageData = m_buffer.left(index);
        m_buffer.remove(0, index + 1);

        QJsonDocument doc = QJsonDocument::fromJson(messageData);
        if (doc.isObject())
        {
            QJsonObject obj = doc.object();

            // Пропускаем heartbeat сообщения
            if (obj["type"].toString() != "HEARTBEAT")
            {
                emit messageReceived(obj);
            }
        }
        else
        {
            qWarning() << "Invalid JSON received:" << messageData;
            sendError("Invalid JSON format");
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::onDisconnected()
{
    emit clientDisconnected();
    deleteLater();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ProtocolHandler::onHeartbeatTimeout()
{
    // Отправляем heartbeat если подключение активно
    if (isConnected())
    {
        QJsonObject heartbeat;
        heartbeat["type"] = "HEARTBEAT";
        heartbeat["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        sendMessage(heartbeat);

        // Проверяем активность клиента
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - m_lastActivity > 60000) // 60 seconds timeout
        {
            qWarning() << "Client heartbeat timeout:" << getClientAddress();
            m_socket->disconnectFromHost();
        }
    }
}