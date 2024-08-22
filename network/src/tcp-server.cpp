#include    <tcp-server.h>
#include    <Journal.h>
#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpServer::TcpServer(QObject *parent) : QObject(parent)
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
bool TcpServer::init(QString cfg_path)
{
    Journal::instance()->info("Starting init TCP-server...");

    CfgReader cfg;

    if (!cfg.load(cfg_path))
    {
        Journal::instance()->error("Can't load server config file from " + cfg_path);
        return false;
    }

    QString secName = "Server";
    int tmp = 0;
    if (cfg.getInt(secName, "port", tmp))
    {
        port = static_cast<quint16>(tmp);
    }

    server = new QTcpServer(this);

    connect(server, &QTcpServer::newConnection, this, &TcpServer::slotNewConnection);

    if (!server->isListening())
    {
        if (server->listen(QHostAddress::Any, port))
        {
            Journal::instance()->info(QString("TCP-server listen at port %1").arg(port));
        }
        else
        {
            Journal::instance()->error(QString("Failed start TCP-server at port %1").arg(port));
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int TcpServer::getClientDataBySocket(QTcpSocket *socket)
{
    for (size_t i = 0; i < clients_data.size(); ++i)
    {
        if (clients_data[i].socket == socket)
        {
            return i;
        }
    }

    return -1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::process_client_request(client_data_t &client_data)
{
    switch (client_data.received_data.stype)
    {
    case STYPE_EMPTY_DATA:
        break;

    case STYPE_TOPOLOGY_DATA:
        send_topology_data(client_data);
        break;

    case STYPE_TRAIN_POSITION:
        break;

    case STYPE_CONNECTOR_STATE:
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::send_topology_data(client_data_t &client_data)
{
    network_data_t net_data;
    net_data.stype = STYPE_TOPOLOGY_DATA;
    net_data.data = topology_data;

    client_data.socket->write(net_data.serialize());
    client_data.socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotNewConnection()
{
    client_data_t client_data;

    client_data.socket = server->nextPendingConnection();


    clients_data.push_back(client_data);

    connect(client_data.socket, &QTcpSocket::disconnected,
            this, &TcpServer::slotClientDisconnected);

    connect(client_data.socket, &QTcpSocket::readyRead,
            this, &TcpServer::slotReceive);

    Journal::instance()->info(QString("Connected client with id %1")
                                  .arg(clients_data.size()));

    Journal::instance()->info(QString("Server receive buffer size: %1")
                                  .arg(client_data.socket->readBufferSize()));

    topology_data.clear();
    emit setTopologyData(topology_data);

    if (topology_data.size())
    {
        Journal::instance()->info(QString("Updated topology data size: %1").arg(topology_data.size()));
    }
    else
    {
        Journal::instance()->error("Failed to update topology data");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotClientDisconnected()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket *>(sender());

    int client_idx = getClientDataBySocket(socket);

    if (client_idx < 0)
        return;

    clients_data[client_idx].socket->close();

    clients_data.erase(clients_data.begin() + client_idx);

    Journal::instance()->info(QString("Disconnected client with id %1")
                                  .arg(clients_data.size()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotReceive()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket *>(sender());

    int client_idx = getClientDataBySocket(socket);

    if (client_idx < 0)
        return;

    while (socket->bytesAvailable())
    {
        if (recvBuff.size() == 0)
        {
            recvBuff.append(socket->readAll());

            QBuffer b(&recvBuff);
            b.open(QIODevice::ReadOnly);
            QDataStream stream(&b);

            stream >> wait_data_size;
        }
        else
        {
            recvBuff.append(socket->readAll());
        }
    }

    if (recvBuff.size() >= wait_data_size)
    {
        clients_data[client_idx].received_data.deserialize(recvBuff);
        process_client_request(clients_data[client_idx]);

        recvBuff.clear();
    }
}
