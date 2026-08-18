//------------------------------------------------------------------------------
//
//  Protocol handler for server
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------

#include    "ProtocolHandler.h"
#include    <QDebug>
#include    <QDateTime>

//-----------------------------------------------------------------------------
ProtocolHandler::ProtocolHandler(QTcpSocket* socket, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_lastActivity(QDateTime::currentMSecsSinceEpoch())
    , m_authenticated(false)
{
    connect(m_socket, &QTcpSocket::readyRead, this, &ProtocolHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ProtocolHandler::onDisconnected);

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(30000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &ProtocolHandler::onHeartbeatTimeout);
    m_heartbeatTimer->start();
}

//-----------------------------------------------------------------------------
ProtocolHandler::~ProtocolHandler()
{
    qInfo() << "ProtocolHandler destructor for client:" << getClientAddress();

    if (m_socket)
    {
        m_socket->disconnectFromHost();
        m_socket->close();
        m_socket = nullptr;
    }

    if (m_heartbeatTimer)
    {
        m_heartbeatTimer->stop();
    }
}

//-----------------------------------------------------------------------------
void ProtocolHandler::sendMessage(const QJsonObject& message)
{
    if (!isConnected())
    {
        return;
    }

    QByteArray data = QJsonDocument(message).toJson(QJsonDocument::Compact);
    data.append('\n');
    m_socket->write(data);
    m_socket->flush();
}

//-----------------------------------------------------------------------------
void ProtocolHandler::sendRouteList(const QJsonArray& routes)
{
    QJsonObject message;
    message["type"] = "ROUTE_LIST";
    message["routes"] = routes;
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::sendScenarioList(const QJsonArray& scenarios)
{
    QJsonObject message;
    message["type"] = "SCENARIO_LIST";
    message["scenarios"] = scenarios;
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::sendError(const QString& error)
{
    QJsonObject message;
    message["type"] = "ERROR";
    message["message"] = error;
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::sendSuccess(const QString& message)
{
    QJsonObject response;
    response["type"] = "SUCCESS";
    response["message"] = message;
    sendMessage(response);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::sendStatus(const QJsonObject& status)
{
    QJsonObject message;
    message["type"] = "STATUS";
    message["status"] = status;
    sendMessage(message);
}

//-----------------------------------------------------------------------------
void ProtocolHandler::sendAuthSuccess()
{
    QJsonObject message;
    message["type"] = "AUTH_SUCCESS";
    sendMessage(message);
}

//-----------------------------------------------------------------------------
QString ProtocolHandler::getClientAddress() const
{
    if (m_socket)
    {
        return m_socket->peerAddress().toString() + ":" + QString::number(m_socket->peerPort());
    }
    return QString();
}

//-----------------------------------------------------------------------------
bool ProtocolHandler::isConnected() const
{
    return m_socket && m_socket->state() == QTcpSocket::ConnectedState;
}

//-----------------------------------------------------------------------------
bool ProtocolHandler::isAuthenticated() const
{
    return m_authenticated;
}

//-----------------------------------------------------------------------------
void ProtocolHandler::setAuthenticated(bool authenticated)
{
    m_authenticated = authenticated;
}

//-----------------------------------------------------------------------------
void ProtocolHandler::onReadyRead()
{
    m_lastActivity = QDateTime::currentMSecsSinceEpoch();
    m_buffer.append(m_socket->readAll());

    qDebug() << "Server buffer size:" << m_buffer.size();

    while (true)
    {
        int index = m_buffer.indexOf('\n');
        if (index == -1)
        {
            qDebug() << "No complete message from client";
            break;
        }

        QByteArray messageData = m_buffer.left(index);
        m_buffer.remove(0, index + 1);

        if (messageData.isEmpty())
        {
            continue;
        }

        qDebug() << "Server received:" << messageData;

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

        qDebug() << "Server received type:" << type;

        if (type == "HEARTBEAT")
        {
            continue;
        }

        emit messageReceived(obj);
    }
}

//-----------------------------------------------------------------------------
void ProtocolHandler::onDisconnected()
{
    qInfo() << "Client disconnected:" << getClientAddress();
    emit clientDisconnected();
    // Не удаляем себя здесь, удаление произойдет в ServerCore
    // deleteLater(); // УБРАТЬ!
}

//-----------------------------------------------------------------------------
void ProtocolHandler::onHeartbeatTimeout()
{
    if (isConnected())
    {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - m_lastActivity > 60000)
        {
            qWarning() << "Client heartbeat timeout:" << getClientAddress();
            m_socket->disconnectFromHost();
        }
    }
}
