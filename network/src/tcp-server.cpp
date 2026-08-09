#include    <tcp-server.h>

#include    <CfgReader.h>
#include    <Journal.h>
#include    <simspeed-command.h>

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>
#include    <QTcpSocket>

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

    case STYPE_REQUEST_PLAYERS_INFO:
    {
        QDataStream stream(&client_data.received_data.data, QIODevice::ReadOnly);

        stream >> client_data.players_update_interval;

        /*Journal::instance()->info(QString("Received players update request for #%1 with interval %2")
                                      .arg(client_data.id).arg(client_data.players_update_interval, 5, 'f', 3));*/
        clients_for_players_info_updates.insert(client_data.socket);
        break;
    }
    case STYPE_REQUEST_ROUTE_INFO:
    {
        client_data.received_data.data.clear();

        //Journal::instance()->info(QString("Received route info request for #%1").arg(client_data.id));
        send_route_info(client_data);
        break;
    }
    case STYPE_REQUEST_TOPOLOGY_DATA:
    {
        client_data.received_data.data.clear();

        //Journal::instance()->info(QString("Received topology data request for #%1").arg(client_data.id));
        send_topology_data(client_data);
        // Пока не разделяем структуру топологии
        // и информацию о её текущем состоянии,
        // считаем что копия топологии сразу готова обновляться
        clients_for_topology_updates.insert(client_data.socket);
        break;
    }
/*    case STYPE_REQUEST_TOPOLOGY_UPDATE:
    {
        client_data.received_data.data.clear();

        Journal::instance()->info(QString("Received topology update request for #%1").arg(client_data.id));
        //send_topology_state(client_data);
        clients_for_topology_updates.insert(client_data.socket);
        break;
    }*/
    case STYPE_REQUEST_SIGNALS_DATA:
    {
        client_data.received_data.data.clear();

        //Journal::instance()->info(QString("Received signals data request for #%1").arg(client_data.id));
        send_signals_data(client_data);
        // Пока не разделяем положение сигналов
        // и информацию об их текущем состоянии,
        // считаем что сигналы сразу готовы обновляться
        clients_for_signals_updates.insert(client_data.socket);
        break;
    }
/*    case STYPE_REQUEST_SIGNALS_UPDATE:
    {
        client_data.received_data.data.clear();

        Journal::instance()->info(QString("Received signals update request for #%1").arg(client_data.id));
        //send_signals_state(client_data);
        clients_for_signals_updates.insert(client_data.socket);
        break;
    }*/
    case STYPE_REQUEST_VEHICLES_INFO:
    {
        client_data.received_data.data.clear();

        //Journal::instance()->info(QString("Received vehicles info request for #%1").arg(client_data.id));
        send_vehicles_info(client_data);
        break;
    }
    case STYPE_REQUEST_TRAINS_UPDATE:
    {
        client_data.received_data.data.clear();

        //Journal::instance()->info(QString("Received trains update request for #%1").arg(client_data.id));

        // Добавляем, чтобы потом из модели разослать всем кому нужно
        // при обновлении инфы
        clients_for_trains_updates.insert(client_data.socket);
        send_trains_info(client_data);

        break;
    }
    case STYPE_REQUEST_VEHICLES_POS_UPDATE:
    {
        QDataStream stream(&client_data.received_data.data, QIODevice::ReadOnly);

        stream >> client_data.pos_update_interval;

        /*Journal::instance()->info(QString("Received vehicles pos update request for #%1 with interval %2")
                                      .arg(client_data.id).arg(client_data.pos_update_interval, 5, 'f', 3));*/
        clients_for_vehicles_pos_updates.insert(client_data.socket);
        break;
    }
    case STYPE_REQUEST_VEHICLES_STATE_UPDATE:
    {
        QDataStream stream(&client_data.received_data.data, QIODevice::ReadOnly);

        stream >> client_data.state_update_interval;

        /*Journal::instance()->info(QString("Received vehicles state update request for #%1 with interval %2")
                                      .arg(client_data.id).arg(client_data.state_update_interval, 5, 'f', 3));*/
        clients_for_vehicles_updates.insert(client_data.socket);
        break;
    }
    case STYPE_REQUEST_VEHICLE_CONTROLLED_UPDATE:
    {
        QDataStream stream(&client_data.received_data.data, QIODevice::ReadOnly);

        stream >> client_data.controlled_update_interval;

        /*Journal::instance()->info(QString("Received vehicle controlled update request for #%1 with interval %2")
                                      .arg(client_data.id).arg(client_data.state_update_interval, 5, 'f', 3));*/
        clients_for_vehicle_controlled_updates.insert(client_data.socket);
        break;
    }
    case STYPE_COMMAND_SWITCH_CONTROL:
    {
        Journal::instance()->info(QString("Received change switch state command from #%1")
                                      .arg(client_data.id));
        emit sigSwitchCommand(client_data.received_data.data);
        break;
    }
    case STYPE_COMMAND_SIGNAL_CONTROL:
    {
        /*Journal::instance()->info(QString("Received signal command from #%1")
                                      .arg(client_data.id));*/
        emit sigSignalCommand(client_data.received_data.data);
        break;
    }
    case STYPE_COMMAND_BUILD_ROUTE:
    {
        /*Journal::instance()->info(QString("Received build route command from #%1")
                                      .arg(client_data.id));*/
        emit sigBuildRouteCommand(client_data.received_data.data);
        break;
    }
    case STYPE_COMMAND_TRAIN_ROUTE:
    {
        /*Journal::instance()->info(QString("Received train route command from #%1")
                                      .arg(client_data.id));*/
        emit sigTrainRouteCommand(client_data.received_data.data);
        break;
    }
    case STYPE_COMMAND_SHUNTING_ROUTE:
    {
        /*Journal::instance()->info(QString("Received shunting route command from #%1")
                                      .arg(client_data.id));*/
        emit sigShuntingRouteCommand(client_data.received_data.data);
        break;
    }
    case STYPE_COMMAND_VEHICLE_CONTROL:
    {
        /*Journal::instance()->info(QString("Received vehicle control command from #%1")
                                      .arg(client_data.id));*/
        emit sigVehicleControl(client_data.received_data.data, client_data.id);
        break;
    }
    case STYPE_COMMAND_RENAME_TRAIN:
    {
        QDataStream stream(&client_data.received_data.data, QIODevice::ReadOnly);

        int train_idx = -1;
        QString new_name = "";

        stream >> train_idx;

        if (train_idx < 0)
        {
            //Journal::instance()->error("Rename train: Invalide train index");
            break;
        }

        stream >> new_name;

        if (new_name.isEmpty())
        {
            //Journal::instance()->error("Rename train: Empty new train name");
            break;
        }

        emit sigRenameTrain(train_idx, new_name);

        break;
    }

    case STYPE_COMMAND_SET_SIMULATION_SPEED:
    {
        simspeed_command_t sc;
        sc.deserialize(client_data.received_data.data);

        emit sigSetSimSpeed(sc.speed_factor);

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
void TcpServer::send_trains_info(client_data_t &client_data)
{
    network_data_t net_data;
    net_data.stype = STYPE_TRAINS_UPDATE;
    net_data.data = trains_state;

    client_data.socket->write(net_data.serialize());
    client_data.socket->flush();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::send_data(QTcpSocket* client_socket, network_data_t& net_data)
{
    if (client_socket->state() == QAbstractSocket::ConnectedState)
    {
        client_socket->write(net_data.serialize());
        client_socket->flush();
    }
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

    Journal::instance()->info(QString("Connected #%1 client %2:%3")
                                  .arg(client_data.id)
                                  .arg(client_data.socket->peerAddress().toString())
                                  .arg(client_data.socket->peerPort()));
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

    if (clients_data.contains(socket))
    {
        client_data_t *client_data = &clients_data[socket];

        client_data->socket->close();

        clients_data.remove(socket);
        clients_for_players_info_updates.remove(socket);
        clients_for_topology_updates.remove(socket);
        clients_for_signals_updates.remove(socket);
        clients_for_vehicles_pos_updates.remove(socket);
        clients_for_vehicles_updates.remove(socket);
        clients_for_vehicle_controlled_updates.remove(socket);
        clients_for_trains_updates.remove(socket);

        emit sigResetVehicleControl(client_data->id);

        Journal::instance()->info(QString("Disconnected client #%1")
                                      .arg(client_data->id));
    }
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
            client_data->received_data.deserialize(recvBuff);

            // Обработка принятого сетевого пакета
            process_client_request(*client_data);

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
void TcpServer::slotSendSwitchState(QByteArray sw_state)
{
    if (clients_for_topology_updates.empty())
    {
        return;
    }

    network_data_t net_data;
    net_data.stype = STYPE_SWITCH_UPDATE;
    net_data.data = sw_state;

    for (auto client_socket : clients_for_topology_updates)
    {
        send_data(client_socket, net_data);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotSendTrajBusyState(QByteArray busy_state)
{
    if (clients_for_topology_updates.empty())
    {
        return;
    }

    network_data_t net_data;
    net_data.stype = STYPE_TRAJ_BUSY_UPDATE;
    net_data.data = busy_state;

    for (auto client_socket : clients_for_topology_updates)
    {
        send_data(client_socket, net_data);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::slotUpdateSignal(QByteArray signal_data)
{
    if (clients_for_signals_updates.empty())
    {
        return;
    }

    network_data_t net_data;
    net_data.stype = STYPE_SIGNAL_UPDATE;
    net_data.data = signal_data;

    for (auto client_socket : clients_for_signals_updates)
    {
        send_data(client_socket, net_data);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::updatePlayers(QByteArray players_data, double t)
{
    if (clients_for_players_info_updates.empty())
    {
        return;
    }

    network_data_t net_data;
    net_data.stype = STYPE_PLAYERS_UPDATE;
    net_data.data = players_data;

    for (auto client_socket : clients_for_players_info_updates)
    {
        double prev_t = clients_data[client_socket].players_update_prev_time;
        if ((t - prev_t) > clients_data[client_socket].players_update_interval)
        {
            clients_data[client_socket].players_update_prev_time = t;
            send_data(client_socket, net_data);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::updateVehiclesPos(QByteArray vehicles_pos, double t)
{
    if (clients_for_vehicles_pos_updates.empty())
    {
        return;
    }

    network_data_t net_data;
    net_data.stype = STYPE_VEHICLES_POS_UPDATE;
    net_data.data = vehicles_pos;

    for (auto client_socket : clients_for_vehicles_pos_updates)
    {
        double prev_t = clients_data[client_socket].pos_update_prev_time;
        if ((t - prev_t) > clients_data[client_socket].pos_update_interval)
        {
            clients_data[client_socket].pos_update_prev_time = t;
            send_data(client_socket, net_data);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::updateVehiclesState(QByteArray vehicles_state, double t)
{
    if (clients_for_vehicles_updates.empty())
    {
        return;
    }

    network_data_t net_data;
    net_data.stype = STYPE_VEHICLES_STATE_UPDATE;
    net_data.data = vehicles_state;

    // Сохраняем эти данные для выдачи по запросу
    this->vehicles_state = vehicles_state;

    for (auto client_socket : clients_for_vehicles_updates)
    {
        double prev_t = clients_data[client_socket].state_update_prev_time;
        if ((t - prev_t) > clients_data[client_socket].state_update_interval)
        {
            clients_data[client_socket].state_update_prev_time = t;
            send_data(client_socket, net_data);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::updateVehicleControlled(QByteArray vehicles_state, int client_id, double t)
{
    if (clients_for_vehicle_controlled_updates.empty())
    {
        return;
    }

    network_data_t net_data;
    net_data.stype = STYPE_VEHICLE_CONTROLLED_UPDATE;
    net_data.data = vehicles_state;

    for (auto client_socket : clients_for_vehicle_controlled_updates)
    {
        if (clients_data[client_socket].id != client_id)
        {
            continue;
        }
        double prev_t = clients_data[client_socket].controlled_update_prev_time;
        if ((t - prev_t) > clients_data[client_socket].controlled_update_interval)
        {
            clients_data[client_socket].controlled_update_prev_time = t;
            send_data(client_socket, net_data);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TcpServer::updateTrainsInfo(QByteArray trains_state)
{
    if (clients_for_trains_updates.empty())
    {
        return;
    }

    network_data_t net_data;
    net_data.stype = STYPE_TRAINS_UPDATE;
    net_data.data = trains_state;

    // Сохраняем эти данные для выдачи по запросу
    this->trains_state = trains_state;

    for (auto client_socket : clients_for_trains_updates)
    {
        send_data(client_socket, net_data);
    }
}
