//------------------------------------------------------------------------------
//
//      TCP-server
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief TCP-server
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#include    "tcp-server.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpServer::TcpServer(QObject *parent) : QObject(parent)
  , server(new QTcpServer())
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpServer::~TcpServer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TcpServer::init()
{
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TcpServer::start()
{
    if (server == Q_NULLPTR)
        return false;

    connect(server, &QTcpServer::newConnection,
            this, &TcpServer::onClientConnection);

    if (!server->listen(QHostAddress::Any, tcp_config.port))
    {
        emit logMessage(QString("ERROR: can't start server. Unable listering on port %1")
                        .arg(tcp_config.port));
        return false;
    }

    emit logMessage(QString("OK: server is started on port %1").arg(tcp_config.port));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::onClientConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();

    Client *client = new Client();
    client->setSocket(socket);
    client->setID(socket->socketDescriptor());

    clients.insert(socket, client);

    connect(socket, &QTcpSocket::readyRead,
            this, &TcpServer::onReceive);

    connect(socket, &QTcpSocket::disconnected,
            this, &TcpServer::onClientDisconnection);

    emit logMessage(QString("OK: Connected client id = %1")
                    .arg(static_cast<int>(socket->socketDescriptor())));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::onReceive()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());

    QByteArray data = socket->readAll();

    emit sendDataToTrain(data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::onClientDisconnection()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());

    using ClientPtr = QScopedPointer<Client>;

    ClientPtr client(clients.value(socket, Q_NULLPTR));

    if (client.isNull())
        return;

    clients.remove(socket);

    emit logMessage(QString("OK: Disconnected client id = %1")
                    .arg(static_cast<int>(socket->socketDescriptor())));
}
