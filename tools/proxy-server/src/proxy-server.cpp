#include    <proxy-server.h>

#include    <CfgReader.h>
#include    <Journal.h>

#include    <QByteArray>
#include    <QDateTime>
#include    <QMap>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProxyServer::ProxyServer(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProxyServer::~ProxyServer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProxyServer::init(QString cfg_path)
{
    Journal::instance()->info("Starting init TCP-proxy...");

    CfgReader cfg;

    if (!cfg.load(cfg_path))
    {
        Journal::instance()->error("Can't load proxy config file from " + cfg_path);
        return false;
    }

    QString secName = "Proxy";

    int tmp = 0;
    if (cfg.getInt(secName, "listen_port", tmp))
    {
        listen_port = static_cast<quint16>(tmp);
    }

    cfg.getString(secName, "target_host", target_host);

    if (cfg.getInt(secName, "target_port", tmp))
    {
        target_port = static_cast<quint16>(tmp);
    }

    if (cfg.getInt(secName, "max_connections", tmp))
    {
        max_connections = tmp;
    }

    if (cfg.getInt(secName, "rate_limit_per_sec", tmp))
    {
        rate_limit_per_sec = tmp;
    }

    if (cfg.getInt(secName, "idle_timeout_ms", tmp))
    {
        idle_timeout_ms = tmp;
    }

    tmp = 0;
    if (cfg.getInt(secName, "max_frame_size", tmp))
    {
        max_frame_size = static_cast<quint32>(tmp);
    }

    server = new QTcpServer(this);

    connect(server, &QTcpServer::newConnection, this, &ProxyServer::slotNewConnection);

    if (server->listen(QHostAddress::Any, listen_port))
    {
        Journal::instance()->info(QString("TCP-proxy listen at port %1, "
                                          "forward to %2:%3")
                                  .arg(listen_port).arg(target_host).arg(target_port));
    }
    else
    {
        Journal::instance()->error(QString("Failed start TCP-proxy at port %1")
                                      .arg(listen_port));
        return false;
    }

    Journal::instance()->info(QString("Limits: max_connections=%1, "
                                      "rate_limit_per_sec=%2, "
                                      "idle_timeout_ms=%3, max_frame_size=%4")
                              .arg(max_connections).arg(rate_limit_per_sec)
                              .arg(idle_timeout_ms).arg(max_frame_size));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProxyServer::isRateLimited()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Удаляем устаревшие отметки (старше 1 секунды)
    while (!connection_times.isEmpty() && (now - connection_times.first()) > 1000)
    {
        connection_times.removeFirst();
    }

    if (connection_times.size() >= rate_limit_per_sec)
    {
        return true;
    }

    connection_times.append(now);

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::slotNewConnection()
{
    QTcpSocket *client_socket = server->nextPendingConnection();

    // Проверка лимита одновременных соединений
    if (connections.size() >= max_connections)
    {
        Journal::instance()->warning("Proxy: limit of concurrent connections reached");
        client_socket->abort();
        return;
    }

    // Проверка ограничения скорости новых подключений
    if (isRateLimited())
    {
        Journal::instance()->warning("Proxy: connection rate limit reached");
        client_socket->abort();
        return;
    }

    qInfo() << QString("Proxy: new connection from %1:%2")
                    .arg(client_socket->peerAddress().toString())
                    .arg(client_socket->peerPort());

    QTcpSocket *target_socket = new QTcpSocket(this);

    qInfo() << QString("Proxy: connecting to target %1:%2")
                    .arg(target_host).arg(target_port);

    connections.insert(client_socket, target_socket);
    last_activity.insert(client_socket, QDateTime::currentMSecsSinceEpoch());

    // Таймер простоя соединения
    QTimer *idle_timer = new QTimer(this);
    idle_timer->setInterval(idle_timeout_ms);
    idle_timer->setSingleShot(true);
    connect(idle_timer, &QTimer::timeout, this, &ProxyServer::slotIdleTimeout);
    idle_timers.insert(client_socket, idle_timer);
    idle_timer->start();

    // Приём данных от внешнего клиента и передача их целевому серверу
    connect(client_socket, &QTcpSocket::readyRead,
            this, &ProxyServer::slotForwardToTarget);

    connect(client_socket, &QTcpSocket::disconnected,
            this, &ProxyServer::slotDisconnected);

    // Приём данных от целевого сервера и передача их внешнему клиенту
    connect(target_socket, &QTcpSocket::readyRead,
            this, &ProxyServer::slotForwardToClient);

    connect(target_socket, &QTcpSocket::disconnected,
            this, &ProxyServer::slotDisconnected);

    // Буфер данных на время соединения с таргетом
    pending_buffers.insert(client_socket, QByteArray());

    // Соединение с таргетом установлено — отправляем накопленные данные
    connect(target_socket, &QTcpSocket::connected,
            this, [this, client_socket]()
    {
        qInfo() << "Proxy: target connected, flushing pending buffer";
        flushPendingBuffer(client_socket);
    });

    // Отказ соединения с целевым сервером
    connect(target_socket, &QTcpSocket::errorOccurred,
            this, [this, client_socket](QAbstractSocket::SocketError)
    {
        qInfo() << "Proxy: target connection error, closing client";
        closeConnection(client_socket);
    });

    target_socket->connectToHost(QHostAddress(target_host), target_port);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::slotForwardToTarget()
{
    QTcpSocket *client_socket = qobject_cast<QTcpSocket *>(sender());

    if (!client_socket)
    {
        return;
    }

    // Проверка размера кадра (защита от половинной атаки)
    if (client_socket->bytesAvailable() > max_frame_size)
    {
        Journal::instance()->warning("Proxy: frame size limit exceeded, closing connection");
        closeConnection(client_socket);
        return;
    }

    auto it = connections.find(client_socket);

    if (it == connections.end())
    {
        return;
    }

    QTcpSocket *target_socket = it.value();

    if (target_socket == nullptr)
    {
        return;
    }

    // Если таргет ещё не подключился
    if (target_socket->state() != QAbstractSocket::ConnectedState)
    {
        // Если соединение когда-то было, но оборвалось — закрываем клиента
        if (target_socket->state() == QAbstractSocket::UnconnectedState &&
            !pending_buffers[client_socket].isEmpty())
        {
            Journal::instance()->warning("Proxy: target disconnected, closing client");
            closeConnection(client_socket);
            return;
        }

        auto bit = pending_buffers.find(client_socket);
        if (bit != pending_buffers.end())
        {
            QByteArray data = client_socket->readAll();
            if (bit.value().size() + data.size() <= max_frame_size)
            {
                bit.value().append(data);
            }
            else
            {
                Journal::instance()->warning("Proxy: pending buffer overflow, closing connection");
                closeConnection(client_socket);
            }
        }
        return;
    }

    qInfo() << QString("Proxy: forwarding %1 bytes to target")
                    .arg(client_socket->bytesAvailable());
    target_socket->write(client_socket->readAll());
    updateActivity(client_socket);
    resetIdleTimer(client_socket);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::slotForwardToClient()
{
    QTcpSocket *target_socket = qobject_cast<QTcpSocket *>(sender());

    if (!target_socket)
    {
        return;
    }

    // Находим внешний сокет, связанный с этим целевым сокетом
    auto it = connections.begin();

    while (it != connections.end())
    {
        if (it.value() == target_socket)
        {
            QTcpSocket *client_socket = it.key();

            // Проверка размера кадра
            if (target_socket->bytesAvailable() > max_frame_size)
            {
                Journal::instance()->warning("Proxy: frame size limit exceeded, closing connection");
                closeConnection(client_socket);
                return;
            }

            if (client_socket->state() == QAbstractSocket::ConnectedState)
            {
                QByteArray data = target_socket->readAll();
                qInfo() << QString("Proxy: forwarding %1 bytes to client").arg(data.size());
                if (!data.isEmpty())
                {
                    client_socket->write(data);
                    resetIdleTimer(client_socket);
                }
            }

            return;
        }

        ++it;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::flushPendingBuffer(QTcpSocket *client_socket)
{
    auto bit = pending_buffers.find(client_socket);
    if (bit == pending_buffers.end())
    {
        qInfo() << "Proxy: flushPendingBuffer - no pending buffer for this socket";
        return;
    }
    if (bit.value().isEmpty())
    {
        qInfo() << "Proxy: flushPendingBuffer - buffer empty, nothing to flush";
        return;
    }

    auto cit = connections.find(client_socket);
    if (cit == connections.end() || cit.value() == nullptr)
    {
        return;
    }

    cit.value()->write(bit.value());
    bit.value().clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::resetIdleTimer(QTcpSocket *client_socket)
{
    auto it = idle_timers.find(client_socket);

    if (it != idle_timers.end())
    {
        it.value()->start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::updateActivity(QTcpSocket *client_socket)
{
    last_activity[client_socket] = QDateTime::currentMSecsSinceEpoch();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::slotDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());

    if (!socket)
    {
        return;
    }

    // Выясняем пару (клиент, целевой сервер) и закрываем обе стороны
    QTcpSocket *client_socket = nullptr;

    if (connections.contains(socket))
    {
        client_socket = socket;
    }
    else
    {
        // socket - целевой; ищем его ключ
        for (auto mit = connections.begin(); mit != connections.end(); ++mit)
        {
            if (mit.value() == socket)
            {
                client_socket = mit.key();
                break;
            }
        }
    }

    if (client_socket == nullptr)
    {
        return;
    }

    closeConnection(client_socket);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::slotIdleTimeout()
{
    QTimer *timer = qobject_cast<QTimer *>(sender());

    if (!timer)
    {
        return;
    }

    // Находим сокет, которому принадлежит таймер
    auto it = idle_timers.begin();

    while (it != idle_timers.end())
    {
        if (it.value() == timer)
        {
            Journal::instance()->warning("Proxy: connection closed by idle timeout");
            closeConnection(it.key());
            return;
        }

        ++it;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::closeConnection(QTcpSocket *client_socket)
{
    if (client_socket == nullptr)
    {
        return;
    }

    auto it = connections.find(client_socket);

    if (it == connections.end())
    {
        // Соединение уже удалено - возможен повторный вызов
        return;
    }

    QTcpSocket *target_socket = it.value();

    connections.remove(client_socket);

    // Очистка буфера отложенных данных
    pending_buffers.remove(client_socket);

    // Очистка времени последней активности
    last_activity.remove(client_socket);

    qInfo() << "Proxy: connection closed";

    // Принудительно закрываем оба сокета, чтобы клиент
    // немедленно получил RST и переподключился
    if (target_socket && target_socket->isOpen())
    {
        target_socket->abort();
    }

    if (client_socket->isOpen())
    {
        client_socket->abort();
    }

    // Остановка и удаление таймера простоя
    auto tit = idle_timers.find(client_socket);
    if (tit != idle_timers.end())
    {
        tit.value()->stop();
        tit.value()->deleteLater();
        idle_timers.remove(client_socket);
    }

    client_socket->deleteLater();
    if (target_socket)
    {
        target_socket->deleteLater();
    }
}
