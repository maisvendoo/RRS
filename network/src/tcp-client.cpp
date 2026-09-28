#include    <tcp-client.h>

#include    <CfgReader.h>
#include    <simspeed-command.h>

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>
#include    <QNetworkProxy>
#include    <QTcpSocket>

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
    connect(socket, &QTcpSocket::disconnected, this, &TcpClient::slotDisconnect);
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
    if (!canSend()) return;

    network_data_t request;
    request.stype = stype;

    QDataStream stream(&request.data, QIODevice::WriteOnly);

    stream << update_interval;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendTrainProfileRequest(double update_interval, double backward_m, double forward_m)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_REQUEST_TRAIN_PROFILE_UPDATE;

    QDataStream stream(&request.data, QIODevice::WriteOnly);

    stream << update_interval;
    stream << backward_m;
    stream << forward_m;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendSwitchCommand(QByteArray switch_command)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_SWITCH_CONTROL;
    request.data = switch_command;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendSignalCommand(QByteArray signal_command)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_SIGNAL_CONTROL;
    request.data = signal_command;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendBuildRouteCommand(QByteArray route_command)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_BUILD_ROUTE;
    request.data = route_command;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendTrainRouteCommand(QByteArray route_command)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_TRAIN_ROUTE;
    request.data = route_command;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendShuntingRouteCommand(QByteArray route_command)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_SHUNTING_ROUTE;
    request.data = route_command;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendVehicleControl(QByteArray vehicle_control_by_keyboard)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_VEHICLE_CONTROL;
    request.data = vehicle_control_by_keyboard;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendNewTrainName(int train_idx, const QString &new_name)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_RENAME_TRAIN;

    QDataStream stream(&request.data, QIODevice::WriteOnly);

    stream << train_idx;
    stream << new_name;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendReverseTrain(int train_idx)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_REVERSE_TRAIN;

    QDataStream stream(&request.data, QIODevice::WriteOnly);

    stream << train_idx;

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::sendSimSpeedCommand(int speed_factor)
{
    if (!canSend()) return;

    network_data_t request;
    request.stype = STYPE_COMMAND_SET_SIMULATION_SPEED;

    simspeed_command_t sc;
    sc.speed_factor = speed_factor;
    request.data = sc.serialize();

    socket->write(request.serialize());
    socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TcpClient::isConnected() const
{
    if (socket == nullptr)
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
bool TcpClient::canSend() const
{
    return socket != nullptr &&
           socket->state() == QAbstractSocket::ConnectedState;
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

    case STYPE_STATIONS_DATA:
        emit setStationsData(net_data.data);
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
/*
    case STYPE_PLAYERS_INFO:
        emit setPlayersInfo(net_data.data);
        break;
*/
    case STYPE_PLAYERS_UPDATE:
        emit setPlayersUpdate(net_data.data);
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

    case STYPE_TRAINS_UPDATE:
        emit setTrainInfo(net_data.data);
        break;

    case STYPE_TRAIN_PROFILE_UPDATE:
        emit setTrainProfile(net_data.data);
        break;

    case STYPE_TOPOLOGY_MODULES:
        emit setTopologyModules(net_data.data);
        break;

    case STYPE_TOPOLOGY_MODULE_UPDATE:
        emit setTopologyModuleUpdate(net_data.data);
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
    connection_attempts = 0;
    emit connected();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotDisconnect()
{
    emit sendLogMessage("Disconnected from the server. Try to reconnect...");
    connection_attempts = 0;
    socket->abort();
    connectionTimer->start();
    emit disconnected();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotOnConnectionTimeout()
{
    ++connection_attempts;

    if (connection_attempts > MAX_CONNECTION_ATTEMPTS)
    {
        connectionTimer->stop();
        emit sendLogMessage("Failed to connect after " +
                            QString::number(MAX_CONNECTION_ATTEMPTS) +
                            " attempts. Giving up.");
        emit connectionAbandoned();
        return;
    }

    this->connectToServer(tcp_config);

    if (tcp_config.show_server_addr)
    {
        emit sendLogMessage("Try connect to " +
                            tcp_config.host_addr + ":" +
                            QString::number(tcp_config.port) +
                            " server... (attempt " +
                            QString::number(connection_attempts) + ")");
    }
    else
    {
        emit sendLogMessage("Try connect to server... (attempt " +
                            QString::number(connection_attempts) + ")");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotReceive()
{
    if (!canSend()) return;

    while (socket->bytesAvailable())
    {
        recvBuff.append(socket->readAll());
    }

    // Если прислали данных не меньше, чем стандартное начало пакета - будем читать
    while (recvBuff.size() >= (sizeof(wait_data_size) + sizeof(StructureType)))
    {
        // Если ждём новый пакет - читаем его ожидаемый размер
        if (is_first_data)
        {
            QDataStream stream(&recvBuff, QIODevice::ReadOnly);

            stream >> wait_data_size;

            is_first_data = false;
        }

        // Если прислали данных не меньше, чем ожидается - забираем их
        if (recvBuff.size() >= wait_data_size)
        {
            // Десериализуем принятые данные в структуру сетевого пакета
            received_data.deserialize(recvBuff);

            // Обработка принятого сетевого пакета
            process_received_data(received_data);

            // Оставляем в буфере только непрочитанный хвост
            recvBuff = recvBuff.mid(wait_data_size);

            // Снова ждём новый пакет
            is_first_data = true;
        }
        else
        {
            // Данных пока прислали недостаточно - выходим, ждём следующих
            break;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpClient::slotAcceptError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    if (socket)
    {
        emit sendLogMessage("Socket error: " + socket->errorString());
    }
}
