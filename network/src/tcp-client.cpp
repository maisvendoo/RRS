#include    <tcp-client.h>
#include    <CfgReader.h>
#include    <iostream>
#include    <QTcpSocket>
#include    <QNetworkProxy>

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
bool TcpClient::init(const tcp_config_t &tcp_config)
{
    this->tcp_config = tcp_config;
    connectionTimer = new QTimer(this);
    connectionTimer->setInterval(tcp_config.reconnect_interval);
    connect(connectionTimer, &QTimer::timeout, this, &TcpClient::slotOnConnectionTimeout);

    socket = new QTcpSocket(this);
    in.setDevice(socket);
    in.setVersion(QDataStream::Qt_4_0);

    connect(socket, &QTcpSocket::errorOccurred, this, &TcpClient::slotAcceptError);
    connect(socket, &QTcpSocket::connected, this, &TcpClient::slotConnect);
    connect(socket, &QTcpSocket::destroyed, this, &TcpClient::slotDisconnect);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::slotReceive);

    socket->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));

    connectionTimer->start();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendRequest(StructureType stype, double update_interval)
{
    network_data_t request;
    request.stype = stype;

    QBuffer buff(&request.data);
    buff.open(QIODevice::WriteOnly);
    QDataStream stream(&buff);

    stream << update_interval;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendSwitchState(QString conn_name, int state_fwd, int state_bwd)
{
    network_data_t request;
    request.stype = STYPE_COMMAND_SWITCH_STATE;

    QBuffer buff(&request.data);
    buff.open(QIODevice::WriteOnly);
    QDataStream stream(&buff);

    stream << conn_name << state_fwd << state_bwd;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendSignalState(QString conn_name, int sig_dir, bool open)
{
    network_data_t request;

    if (open)
    {
        request.stype = STYPE_COMMAND_OPEN_SIGNAL;
    }
    else
    {
        request.stype = STYPE_COMMAND_CLOSE_SIGNAL;
    }

    QBuffer buff(&request.data);
    buff.open(QIODevice::WriteOnly);
    QDataStream stream(&buff);

    stream << conn_name;
    stream << sig_dir;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendVehicleControl(QByteArray vehicle_control_by_keyboard)
{
    network_data_t request;
    request.stype = STYPE_COMMAND_VEHICLE_CONTROL;
    request.data = vehicle_control_by_keyboard;

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

    if (socket->state() == QAbstractSocket::ConnectedState)
    {
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::connectToServer(const tcp_config_t &tcp_config)
{
    socket->abort();
    socket->connectToHost(QHostAddress(tcp_config.host_addr),
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
    case STYPE_ROUTE_INFO:
        emit setRouteInfo(net_data.data);
        break;

    case STYPE_TOPOLOGY_DATA:
        emit setTopologyData(net_data.data);
        break;
/*
    case STYPE_TOPOLOGY_STATE:
        emit setTopologyState(net_data.data);
        break;
*/
    case STYPE_SWITCH_UPDATE:
        emit setSwitchState(net_data.data);
        break;

    case STYPE_TRAJ_BUSY_UPDATE:
        emit setTrajBusyState(net_data.data);
        break;

    case STYPE_SIGNALS_DATA:
        emit setSignalsData(net_data.data);
        break;
/*
    case STYPE_SIGNALS_STATE:
        emit setSignalsState(net_data.data);
        break;
*/
    case STYPE_SIGNAL_UPDATE:
        emit updateSignal(net_data.data);
        break;

    case STYPE_VEHICLES_INFO:
        emit setVehiclesInfo(net_data.data);
        break;

    case STYPE_VEHICLES_POS_UPDATE:
        emit setVehiclesPositions(net_data.data);
        break;

    case STYPE_VEHICLES_STATE_UPDATE:
        emit setVehiclesData(net_data.data);
        break;

    case STYPE_VEHICLE_CONTROLLED_UPDATE:
        emit setVehicleControlled(net_data.data);
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
    //Journal::instance()->info("Disconnected from the server...");
    emit sendLogMessage("Disconnected from the server");
    socket->abort();
    connectionTimer->start();
    emit disconnect();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotOnConnectionTimeout()
{
    this->connectToServer(tcp_config);
    //Journal::instance()->info("Try connect to server...");

    emit sendLogMessage("Try connect to server " +
                        tcp_config.host_addr + ":" +
                        QString::number(tcp_config.port));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotReceive()
{
    while (socket->bytesAvailable())
    {
        recvBuff.append(socket->readAll());
    }

    while (recvBuff.size() > 0)
    {
        if (is_first_data)
        {
            QBuffer b(&recvBuff);
            b.open(QIODevice::ReadOnly);
            QDataStream stream(&b);

            stream >> wait_data_size;

            is_first_data = false;
        }

        if (recvBuff.size() > wait_data_size)
        {
            // Десериализуем принятые данные в структуру сетевого пакета
            received_data.deserialize(recvBuff);

            // Обработка принятого сетевого пакета
            process_received_data(received_data);

            is_first_data = true;
        }
        else
        {
            break;
        }
    }
}

void TcpClient::slotAcceptError(QAbstractSocket::SocketError error)
{

}
