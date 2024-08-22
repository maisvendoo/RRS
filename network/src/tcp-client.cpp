#include    <tcp-client.h>
#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpClient::TcpClient(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TcpClient::~TcpClient()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TcpClient::init(QString cfg_name)
{
    CfgReader cfg;

    if (!cfg.load(cfg_name))
    {
        return false;
    }

    QString secName = "Client";
    cfg.getString(secName, "HostAddr", tcp_config.host_addr);

    int tmp = 0;

    if (cfg.getInt(secName, "port", tmp))
    {
        tcp_config.port = static_cast<quint16>(tmp);
    }

    cfg.getInt(secName, "ReconnectInteval", tcp_config.reconnect_interval);

    connectionTimer = new QTimer(this);
    connectionTimer->setInterval(tcp_config.reconnect_interval);
    connect(connectionTimer, &QTimer::timeout, this, &TcpClient::slotOnConnectionTimeout);

    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &TcpClient::slotConnect);
    connect(socket, &QTcpSocket::destroyed, this, &TcpClient::slotDisconnect);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::slotReceive);

    connectionTimer->start();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendRequest(StructureType stype)
{
    network_data_t request;
    request.stype = stype;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TcpClient::isConnected() const
{
    if (socket == Q_NULLPTR)
    {
        return false;
    }

    return socket->state() == QTcpSocket::ConnectedState;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::connectToServer(const tcp_config_t &tcp_config)
{
    socket->abort();
    socket->connectToHost(tcp_config.host_addr,
                          tcp_config.port,
                          QIODevice::ReadWrite);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::process_received_data(network_data_t &net_data)
{
    switch (net_data.stype)
    {
    case STYPE_TOPOLOGY_DATA:
    {

        qsizetype size = net_data.data.size();

        emit setTopologyData(net_data.data);
        break;
    }
    case STYPE_TRAIN_POSITION:
        emit setSimulatorData(net_data.data);
        break;

    default:

        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotConnect()
{
    connectionTimer->stop();
    emit connected();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotDisconnect()
{
    socket->abort();
    connectionTimer->start();
    emit disconnect();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotOnConnectionTimeout()
{
    if (!isConnected())
    {
        this->connectToServer(tcp_config);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotReceive()
{
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
        received_data.deserialize(recvBuff);
        process_received_data(received_data);

        recvBuff.clear();
    }
}
