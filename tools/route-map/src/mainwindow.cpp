#include    <mainwindow.h>
#include    <ui_mainwindow.h>

#include    <CfgReader.h>
#include    <QPainter>
#include    <connector.h>
#include    <QMenu>
#include    <switch.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(route_map_command_line_t &cmd_line, QWidget *parent): QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setWindowTitle(tr("route-map"));

    ui->setupUi(this);

    connect(tcp_client, &TcpClient::connected,
            this, &MainWindow::slotConnectedToSimulator);

    connect(tcp_client, &TcpClient::disconnected,
            this, &::MainWindow::slotDisconnectedFromSimulator);

    connect(tcp_client, &TcpClient::setVehiclesInfo,
            this, &MainWindow::slotGetVehicleInfoData);

    connect(tcp_client, &TcpClient::setTopologyData,
            this, &MainWindow::slotGetTopologyData);

    connect(tcp_client, &TcpClient::setSignalsData,
            this, &MainWindow::slotGetSignalsData);

    connect(tcp_client, &TcpClient::setPlayersUpdate,
            this, &MainWindow::slotGetPlayersData);

    connect(tcp_client, &TcpClient::setVehiclesPositions,
            this, &MainWindow::slotGetVehiclePosData);

    connect(tcp_client, &TcpClient::setSwitchState,
            this, &MainWindow::slotGetSwitchState);

    connect(tcp_client, &TcpClient::setTrajBusyState,
            this, &MainWindow::slotGetTrajBusyState);

    connect(tcp_client, &TcpClient::updateSignal,
            this, &MainWindow::slotUpdateSignal);

    connect(tcp_client, &TcpClient::sendLogMessage,
            this, &MainWindow::slotRecvLogMessage);

    map = new MapWidget(ui->Map);
    map->stations = topology->getStationsList();
    map->traj_list = topology->getTrajectoriesList();
    map->conn_list = topology->getConnectorsList();
    map->signals_data = signals_data;
    map->train_data = &train_data;
    map->vehicles_half_length = &vehicles_half_length;
    map->players_data = &players_data;

    load_config("../cfg/route-map-tcp.xml");

    overrideByCommandLine(cmd_line);

    tcp_client->init(tcp_config);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::load_config(const QString &cfg_name)
{
    CfgReader cfg;

    if (!cfg.load(cfg_name))
    {
        return;
    }

    QString secName = "Client";
    cfg.getString(secName, "HostAddr", tcp_config.host_addr);

    int tmp = 0;

    if (cfg.getInt(secName, "port", tmp))
    {
        tcp_config.port = static_cast<quint16>(tmp);
    }

    cfg.getInt(secName, "ReconnectInteval", tcp_config.reconnect_interval);
    cfg.getInt(secName, "VehiclesPosUpdateInterval", vehicles_pos_update_interval);
    cfg.getInt(secName, "PlayersUpdateInterval", players_update_interval);

    secName = "RouteMap";
    double tmp_value = 0;
    cfg.getDouble(secName, "SwitchLength", tmp_value);
    map->setSwitchLength(tmp_value);

    tmp_value = 0;
    cfg.getDouble(secName, "SignalRadius", tmp_value);
    map->setSignalRadius(tmp_value);

    tmp_value = 0;
    cfg.getDouble(secName, "SignalOffset", tmp_value);
    map->setSignalOffset(tmp_value);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::overrideByCommandLine(route_map_command_line_t &cmd_line)
{
    if (cmd_line.host_addr.is_present)
    {
        tcp_config.host_addr = cmd_line.host_addr.value;
    }
    if (cmd_line.port.is_present)
    {
        tcp_config.port = cmd_line.port.value;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::paintEvent(QPaintEvent *event)
{
    map->resize(ui->Map->width(), ui->Map->height());
    map->update();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateStations()
{
    ui->mStations->clear();

    int stations_size = topology->getStationsList()->size();
    if (stations_size == 0)
    {
        return;
    }

    for (int i = 0; i < stations_size; ++i)
    {
        topology_station_t ts = topology->getStationsList()->at(i);

        QAction *action_station = new QAction(ts.name);
        ui->mStations->addAction(action_station);

        MapWidget *mw = map;
        int idx = i;
        connect(action_station, &QAction::triggered, this, [mw, idx]{
            mw->slotStationAtCenter(idx);
        });
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updatePlayers()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotConnectedToSimulator()
{
    players_data = simulator_update_players_t();
    train_data = simulator_update_pos_t();
    vehicles_half_length.clear();

    // Запрос серверу на информацию о длинах ПЕ
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_INFO);

    ui->ptLog->appendPlainText(tr("Send request for vehicles info..."));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotDisconnectedFromSimulator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetVehicleInfoData(QByteArray &data)
{
    simulator_vehicles_info_t info;
    info.deserialize(data);

    size_t num = info.vehicles.size();
    ui->ptLog->appendPlainText(QString(tr("Loaded info about %1 vehicles")).arg(num));

    // Сохраняем длины ПЕ для отрисовки
    vehicles_half_length.resize(num);
    for (size_t i = 0; i < num; ++i)
    {
        vehicles_half_length[i] = info.vehicles[i].vehicle_length / 2.0;
    }

    // Запрос серверу на загрузку топологии
    tcp_client->sendRequest(STYPE_REQUEST_TOPOLOGY_DATA);

    ui->ptLog->appendPlainText(tr("Send request for topology loading..."));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetTopologyData(QByteArray &topology_data)
{
    topology->deserialize(topology_data);
    this->setWindowTitle(topology->getRouteName());

    updateStations();
    map->slotStationAtCenter(0);
    map->slotPlayerAtCenter(0);

    if ( (topology->getTrajectoriesList() == Q_NULLPTR) || (topology->getConnectorsList() == Q_NULLPTR) )
    {
        ui->ptLog->appendPlainText(tr("Toplology loading FAILED!!!"));
        return;
    }

    if (topology->getTrajectoriesList()->size() == 0)
    {
        ui->ptLog->appendPlainText(tr("Trajectories list is empty"));
        return;
    }

    if (topology->getConnectorsList()->size() == 0)
    {
        ui->ptLog->appendPlainText(tr("Connectors list is empty"));
        return;
    }

    for (auto sl : map->switch_labels)
    {
        delete sl;
    }
    map->switch_labels.clear();

    for (auto conn : *topology->getConnectorsList())
    {
        SwitchLabel *sw_label = new SwitchLabel(map);
        sw_label->setText(conn->getName());
        sw_label->conn = conn;

        connect(sw_label, &SwitchLabel::popUpMenu, this, &MainWindow::slotSwitchConnectorMenu);

        map->switch_labels.insert(conn->getName(), sw_label);
    }

    ui->ptLog->appendPlainText(tr("Topology loaded successfully!"));

    QString trajectories = QString(tr("Trajectories: %1")).arg(topology->getTrajectoriesList()->size());
    QString connestors = QString(tr("Connectors: %1")).arg(topology->getConnectorsList()->size());

    ui->ptLog->appendPlainText(trajectories);
    ui->ptLog->appendPlainText(connestors);

    // Запрос серверу на загрузку сигналов
    tcp_client->sendRequest(STYPE_REQUEST_SIGNALS_DATA);
    ui->ptLog->appendPlainText(tr("Send request for signals data loading..."));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetSignalsData(QByteArray &sig_data)
{
    signals_data->deserialize(sig_data);

    if (signals_data->line_signals.size() != 0)
    {
        ui->ptLog->appendPlainText(QString(tr("Loaded %1 line signals")).arg(signals_data->line_signals.size()));
    }
    else
    {
        ui->ptLog->appendPlainText(QString(tr("Warning: no line signals data")));
    }

    if (signals_data->enter_signals.size() != 0)
    {
        ui->ptLog->appendPlainText(QString(tr("Loaded %1 enter signals")).arg(signals_data->enter_signals.size()));
    }
    else
    {
        ui->ptLog->appendPlainText(QString(tr("Warning: no enter signals data")));
    }

    if (signals_data->exit_signals.size() != 0)
    {
        ui->ptLog->appendPlainText(QString(tr("Loaded %1 exit signals")).arg(signals_data->exit_signals.size()));
    }
    else
    {
        ui->ptLog->appendPlainText(QString(tr("Warning: no exit signals data")));
    }

    for (auto sl : map->signal_labels_fwd)
    {
        delete sl;
    }
    for (auto sl : map->signal_labels_bwd)
    {
        delete sl;
    }
    map->signal_labels_fwd.clear();
    map->signal_labels_bwd.clear();

    for (auto signal : signals_data->line_signals)
    {
        Connector *conn = topology->getConnectorsList()->value(signal->getConnectorName(), Q_NULLPTR);

        if (conn == Q_NULLPTR)
        {
            continue;
        }

        signal->setConnector(conn);

        SignalLabel *signal_label = new SignalLabel(map);
        signal_label->signal = signal;
        signal_label->setText(signal->getLetter());

        if (signal->getDirection() == 1)
        {
            conn->setSignalFwd(signal);

            map->signal_labels_fwd.insert(conn->getName(), signal_label);
        }

        if (signal->getDirection() == -1)
        {
            conn->setSignalBwd(signal);

            map->signal_labels_bwd.insert(conn->getName(), signal_label);
        }
    }

    for (auto signal : signals_data->enter_signals)
    {
        Connector *conn = topology->getConnectorsList()->value(signal->getConnectorName(), Q_NULLPTR);

        if (conn == Q_NULLPTR)
        {
            continue;
        }

        signal->setConnector(conn);

        SignalLabel *signal_label = new SignalLabel(map);
        signal_label->signal = signal;
        signal_label->setText(signal->getLetter());

        if (signal->getDirection() == 1)
        {
            conn->setSignalFwd(signal);

            map->signal_labels_fwd.insert(conn->getName(), signal_label);
        }

        if (signal->getDirection() == -1)
        {
            conn->setSignalBwd(signal);

            map->signal_labels_bwd.insert(conn->getName(), signal_label);
        }

        connect(signal_label, &SignalLabel::popUpMenu, this, &MainWindow::slotSignalControlMenu);
    }

    for (auto signal : signals_data->exit_signals)
    {
        Connector *conn = topology->getConnectorsList()->value(signal->getConnectorName(), Q_NULLPTR);

        if (conn == Q_NULLPTR)
        {
            continue;
        }

        signal->setConnector(conn);

        SignalLabel *signal_label = new SignalLabel(map);
        signal_label->signal = signal;
        signal_label->setText(signal->getLetter());

        if (signal->getDirection() == 1)
        {
            conn->setSignalFwd(signal);

            map->signal_labels_fwd.insert(conn->getName(), signal_label);
        }

        if (signal->getDirection() == -1)
        {
            conn->setSignalBwd(signal);

            map->signal_labels_bwd.insert(conn->getName(), signal_label);
        }

        connect(signal_label, &SignalLabel::popUpMenu, this, &MainWindow::slotSignalControlMenu);
    }

    // Запрос серверу на регулярное обновление игроков
    tcp_client->sendRequest(STYPE_REQUEST_PLAYERS_INFO,
                            static_cast<double>(players_update_interval) / 1000.0);
    // Запрос серверу на регулярное обновление положений ПЕ
    tcp_client->sendRequest(STYPE_REQUEST_VEHICLES_POS_UPDATE,
                            static_cast<double>(vehicles_pos_update_interval) / 1000.0);
    ui->ptLog->appendPlainText(tr("Send request for continuous vehicles update"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetPlayersData(QByteArray &players_update)
{
    players_data.deserialize(players_update);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetVehiclePosData(QByteArray &sim_data)
{
    train_data.deserialize(sim_data);

    int seconds = static_cast<int>(std::floor(train_data.time));
    int hours = seconds / 3600;
    int minutes = seconds / 60 % 60;
    seconds = seconds % 60;
    QString time_text = QString("Время от начала симуляции: %1 сек (%2 ч %3 м %4 c)")
                           .arg(train_data.time, 8, 'f', 1)
                           .arg(hours, 2)
                           .arg(minutes, 2)
                           .arg(seconds, 2);
    ui->statusbar->showMessage(time_text);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSwitchConnectorMenu()
{
    SwitchLabel *sw_label = dynamic_cast<SwitchLabel *>(sender());
    QString conn_name = sw_label->conn->getName();

    Switch *sw = dynamic_cast<Switch *>(sw_label->conn);
    int state_fwd = sw->getStateFwd();
    int state_bwd = sw->getStateBwd();

    if ((state_fwd == 0) && (state_bwd == 0))
        return;

    TcpClient *tc = tcp_client;

    QMenu *menu = new QMenu(this);

    sw_label->menu = menu;

    if (state_fwd != 0)
    {
        QAction *action_switch_fwd = new QAction(tr("Switch forward"), this);
        action_switch_fwd->setEnabled((sw->getStateFwd() != 2) && (sw->getStateFwd() != -2));
        menu->addAction(action_switch_fwd);

        sw_label->action_switch_fwd = action_switch_fwd;
        connect(action_switch_fwd, &QAction::triggered, sw_label, &SwitchLabel::resetMenu);

        connect(action_switch_fwd, &QAction::triggered, this, [conn_name, state_fwd, state_bwd, tc]{
            tc->sendSwitchState(conn_name, -sign(state_fwd), state_bwd);
        });
    }

    if (state_bwd != 0)
    {
        QAction *action_switch_bwd = new QAction(tr("Switch backward"), this);
        action_switch_bwd->setEnabled((sw->getStateBwd() != 2) && (sw->getStateBwd() != -2));
        menu->addAction(action_switch_bwd);

        sw_label->action_switch_bwd = action_switch_bwd;
        connect(action_switch_bwd, &QAction::triggered, sw_label, &SwitchLabel::resetMenu);

        connect(action_switch_bwd, &QAction::triggered, this, [conn_name, state_fwd, state_bwd, tc]{
            tc->sendSwitchState(conn_name, state_fwd, -sign(state_bwd));
        });
    }

    menu->exec(QCursor::pos());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSignalControlMenu()
{
    SignalLabel *signal_label = dynamic_cast<SignalLabel *>(sender());
    Signal *signal = signal_label->signal;

    TcpClient *tc = tcp_client;

    QMenu *menu = new QMenu(this);

    QAction *open = new QAction(tr("Open"), this);
    menu->addAction(open);

    connect(open, &QAction::triggered, this, [tc, signal]{
        tc->sendSignalState(signal->getConnector()->getName(), signal->getDirection(), true);
    });

    QAction *close = new QAction(tr("Close"), this);
    menu->addAction(close);

    connect(close, &QAction::triggered, this, [tc, signal]{
        tc->sendSignalState(signal->getConnector()->getName(), signal->getDirection(), false);
    });

    menu->exec(QCursor::pos());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetSwitchState(QByteArray &sw_state)
{
    switch_state_t switch_state;
    switch_state.deserialize(sw_state);

    Switch *sw = dynamic_cast<Switch *>(topology->getConnectorsList()->value(switch_state.name, Q_NULLPTR));

    if (sw == Q_NULLPTR)
    {
        return;
    }

    sw->setStateFwd(switch_state.state_fwd);
    sw->setStateBwd(switch_state.state_bwd);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetTrajBusyState(QByteArray &busy_data)
{
    traj_busy_state_t busy_state;
    busy_state.deserialize(busy_data);

    Trajectory *traj = (topology->getTrajectoriesList()->value(busy_state.name, Q_NULLPTR));

    if (traj == Q_NULLPTR)
    {
        return;
    }

    traj->setBusyState(busy_state.is_busy);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotUpdateSignal(QByteArray signal_data)
{
    QBuffer buff(&signal_data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    QString conn_name = "";
    int signal_dir = 0;

    stream >> conn_name;
    stream >> signal_dir;

    if (conn_name.isEmpty())
    {
        return;
    }

    Connector *conn = topology->getConnectorsList()->value(conn_name, Q_NULLPTR);

    if (conn == Q_NULLPTR)
    {
        return;
    }

    if (signal_dir == 1)
    {
        conn->getSignalFwd()->deserialize(signal_data);
    }

    if (signal_dir == -1)
    {
        conn->getSignalBwd()->deserialize(signal_data);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotRecvLogMessage(QString msg)
{
    ui->ptLog->appendPlainText(msg);
}
