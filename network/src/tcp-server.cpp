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
void TcpServer::process_client_request(client_data_t &client_data)
{
    switch (client_data.received_data.stype)
    {

    case STYPE_REQUEST_ROUTE_INFO:
    {
        Journal::instance()->info(QString("Received route info request for %1").arg(client_data.id));
        send_route_info(client_data);
        break;
    }
    case STYPE_REQUEST_TOPOLOGY_DATA:
    {
        Journal::instance()->info(QString("Received topology data request for %1").arg(client_data.id));
        send_topology_data(client_data);
        // Пока не разделяем структуру топологии
        // и информацию о её текущем состоянии,
        // считаем что копия топологии сразу готова обновляться
        clients_for_topology_updates.insert(client_data.socket);
        break;
    }
/*    case STYPE_REQUEST_TOPOLOGY_UPDATE:
    {
        Journal::instance()->info(QString("Received topology update request for %1").arg(client_data.id));
        //send_topology_state(client_data);
        clients_for_topology_updates.insert(client_data.socket);
        break;
    }*/
    case STYPE_REQUEST_SIGNALS_DATA:
    {
        Journal::instance()->info(QString("Received signals data request for %1").arg(client_data.id));
        send_signals_data(client_data);
        // Пока не разделяем положение сигналов
        // и информацию об их текущем состоянии,
        // считаем что сигналы сразу готовы обновляться
        clients_for_signals_updates.insert(client_data.socket);
        break;
    }
/*    case STYPE_REQUEST_SIGNALS_UPDATE:
    {
        Journal::instance()->info(QString("Received signals update request for %1").arg(client_data.id));
        //send_signals_state(client_data);
        clients_for_signals_updates.insert(client_data.socket);
        break;
    }*/
    case STYPE_REQUEST_VEHICLES_INFO:
    {
        Journal::instance()->info(QString("Received vehicles info request for %1").arg(client_data.id));
        send_vehicles_info(client_data);
        break;
    }
    case STYPE_REQUEST_VEHICLES_POS_UPDATE:
    {
        Journal::instance()->info(QString("Received vehicles pos update request for %1").arg(client_data.id));
        clients_for_vehicles_pos_updates.insert(client_data.socket);
        break;
    }
    case STYPE_REQUEST_VEHICLES_STATE_UPDATE:
    {
        Journal::instance()->info(QString("Received vehicles update request for %1").arg(client_data.id));
        clients_for_vehicles_updates.insert(client_data.socket);
        break;
    }
    case STYPE_COMMAND_SWITCH_STATE:
    {
        Journal::instance()->info("Received change switch state request");
        emit setSwitchState(client_data.received_data.data);
        break;
    }
    case STYPE_COMMAND_OPEN_SIGNAL:
    {
        Journal::instance()->info("Received open signal request");
        emit openSignal(client_data.received_data.data);
        break;
    }
    case STYPE_COMMAND_CLOSE_SIGNAL:
    {
        Journal::instance()->info("Received close signal request");
        emit closeSignal(client_data.received_data.data);
        break;
    }
    case STYPE_COMMAND_VEHICLE_CONTROL:
    {
        Journal::instance()->info("Received vehicle control request");
        emit setVehicleControl(client_data.received_data.data);
        break;
    }

    case STYPE_EMPTY_DATA:
    default:

        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::send_route_info(client_data_t &client_data)
{
    network_data_t net_data;
    net_data.stype = STYPE_ROUTE_INFO;
    net_data.data = route_info;

    client_data.socket->write(net_data.serialize());
    client_data.socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::send_topology_data(client_data_t &client_data)
{
    QByteArray data;
    emit requestTopologyData(data);

    network_data_t net_data;
    net_data.stype = STYPE_TOPOLOGY_DATA;
    net_data.data = data;

    client_data.socket->write(net_data.serialize());
    client_data.socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::send_signals_data(client_data_t &client_data)
{
    QByteArray data;
    emit requestSignalsData(data);

    network_data_t net_data;
    net_data.stype = STYPE_SIGNALS_DATA;
    net_data.data = data;

    client_data.socket->write(net_data.serialize());
    client_data.socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::send_vehicles_info(client_data_t &client_data)
{
    network_data_t net_data;
    net_data.stype = STYPE_VEHICLES_INFO;
    net_data.data = vehicles_info;

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

    client_data.id = clients_last_id;

    clients_data.insert(client_data.socket, client_data);

    ++clients_last_id;

    connect(client_data.socket, &QTcpSocket::disconnected,
            this, &TcpServer::slotClientDisconnected);

    connect(client_data.socket, &QTcpSocket::readyRead,
            this, &TcpServer::slotReceive);

    Journal::instance()->info(QString("Connected client with id %1")
                                  .arg(client_data.id));

    Journal::instance()->info(QString("Server receive buffer size: %1")
                                  .arg(client_data.socket->readBufferSize()));
/*
    topology_data.clear();
    emit setTopologyData(topology_data);

    if (topology_data.size() != 0)
    {
        Journal::instance()->info(QString("Updated topology data size: %1").arg(topology_data.size()));
    }
    else
    {
        Journal::instance()->error("Failed to update topology data");
    }

    signals_data.clear();
    emit setSignalsData(signals_data);

    if (signals_data.size() != 0)
    {
        Journal::instance()->info(QString("Updated signals data size %1").arg(signals_data.size()));
    }
    else
    {
        Journal::instance()->error("Failed to update signals data");
    }*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotClientDisconnected()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket *>(sender());

    client_data_t *client_data;

    if (clients_data.contains(socket))
    {
        client_data = &clients_data[socket];
        Journal::instance()->info(QString("Disconnected client with id %1")
                                      .arg(client_data->id));
    }
    else
    {
        return;
    }

    client_data->socket->close();

    clients_data.remove(socket);
    clients_for_topology_updates.remove(socket);
    clients_for_signals_updates.remove(socket);
    clients_for_vehicles_pos_updates.remove(socket);
    clients_for_vehicles_updates.remove(socket);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotReceive()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket *>(sender());

    client_data_t *client_data;

    if (clients_data.contains(socket))
    {
        client_data = &clients_data[socket];
    }
    else
    {
        return;
    }

    while (socket->bytesAvailable())
    {
        if (is_first_data)
        {
            recvBuff.append(socket->readAll());

            QBuffer b(&recvBuff);
            b.open(QIODevice::ReadOnly);
            QDataStream stream(&b);

            stream >> wait_data_size;

            is_first_data = false;
        }
        else
        {
            recvBuff.append(socket->readAll());
        }
    }

    if (recvBuff.size() > wait_data_size)
    {
        client_data->received_data.deserialize(recvBuff);
        process_client_request(*client_data);

        is_first_data = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotSendSwitchState(QByteArray sw_state)
{
    network_data_t net_data;
    net_data.stype = STYPE_SWITCH_UPDATE;
    net_data.data = sw_state;

    for (auto client_socket : clients_for_topology_updates)
    {
        client_socket->write(net_data.serialize());
        client_socket->flush();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotSendTrajBusyState(QByteArray busy_state)
{
    network_data_t net_data;
    net_data.stype = STYPE_TRAJ_BUSY_UPDATE;
    net_data.data = busy_state;

    for (auto client_socket : clients_for_topology_updates)
    {
        client_socket->write(net_data.serialize());
        client_socket->flush();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotUpdateSignal(QByteArray signal_data)
{
    network_data_t net_data;
    net_data.stype = STYPE_SIGNAL_UPDATE;
    net_data.data = signal_data;

    for (auto client_socket : clients_for_signals_updates)
    {
        client_socket->write(net_data.serialize());
        client_socket->flush();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotUpdateVehiclesPos(QByteArray vehicles_pos)
{
    network_data_t net_data;
    net_data.stype = STYPE_SIGNAL_UPDATE;
    net_data.data = vehicles_pos;

    for (auto client_socket : clients_for_vehicles_pos_updates)
    {
        client_socket->write(net_data.serialize());
        client_socket->flush();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotUpdateVehiclesState(QByteArray vehicles_state)
{
    network_data_t net_data;
    net_data.stype = STYPE_SIGNAL_UPDATE;
    net_data.data = vehicles_state;

    for (auto client_socket : clients_for_vehicles_updates)
    {
        client_socket->write(net_data.serialize());
        client_socket->flush();
    }
}
