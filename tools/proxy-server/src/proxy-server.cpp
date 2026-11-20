#include    <proxy-server.h>

#include    <CfgReader.h>
#include    <Journal.h>

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

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProxyServer::slotNewConnection()
{
    QTcpSocket *client_socket = server->nextPendingConnection();

    Journal::instance()->info(QString("Proxy: new connection from %1:%2")
                                .arg(client_socket->peerAddress().toString())
                                .arg(client_socket->peerPort()));

    QTcpSocket *target_socket = new QTcpSocket(this);

    connections.insert(client_socket, target_socket);

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

    // Отказ соединения с целевым сервером
    connect(target_socket, &QTcpSocket::errorOccurred,
            this, [this, client_socket](QAbstractSocket::SocketError)
    {
        client_socket->disconnectFromHost();
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

    auto it = connections.find(client_socket);

    if (it == connections.end())
    {
        return;
    }

    QTcpSocket *target_socket = it.value();

    if (target_socket == nullptr || target_socket->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }

    target_socket->write(client_socket->readAll());
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

            if (client_socket->state() == QAbstractSocket::ConnectedState)
            {
                client_socket->write(target_socket->readAll());
            }

            return;
        }

        ++it;
    }
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
    QTcpSocket *target_socket = nullptr;

    auto it = connections.find(socket);

    if (it != connections.end())
    {
        client_socket = it.key();
        target_socket = it.value();
    }
    else
    {
        // socket - целевой; ищем его ключ
        for (auto mit = connections.begin(); mit != connections.end(); ++mit)
        {
            if (mit.value() == socket)
            {
                client_socket = mit.key();
                target_socket = mit.value();
                break;
            }
        }
    }

    if (client_socket == nullptr || target_socket == nullptr)
    {
        return;
    }

    connections.remove(client_socket);

    Journal::instance()->info("Proxy: connection closed");

    if (client_socket->isOpen())
    {
        client_socket->disconnectFromHost();
    }

    if (target_socket->isOpen())
    {
        target_socket->disconnectFromHost();
    }

    client_socket->deleteLater();
    target_socket->deleteLater();
}
