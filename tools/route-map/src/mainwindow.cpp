#include    <mainwindow.h>
#include    <ui_mainwindow.h>

#include    <CfgReader.h>
#include    <QPainter>
#include    <connector.h>
#include    <QMenu>
#include    <switch.h>
#include    <QInputDialog>

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

    connect(ui->actionShowTrajName, &QAction::triggered,
            this, &MainWindow::slotSetShowTrajStatus);

    connect(tcp_client, &TcpClient::setTrainInfo,
            this, &MainWindow::slotGetTrainsInfo);

    bg = new BackGroundWidget(ui->Map);

    map = new MapWidget(ui->Map);
    map->stations = topology->getStationsList();
    map->traj_list = topology->getTrajectoriesList();
    map->conn_list = topology->getConnectorsList();
    map->signals_data = signals_data;
    map->train_data = &train_data;
    map->vehicles_half_length = &vehicles_half_length;
    map->players_data = &players_data;

    connect(map, &MapWidget::sigOpenTrajectoryMenu,
            this, &MainWindow::slotNearestTrajectoryMenu);

    connect(map, &MapWidget::sigSelectNearestTrajectory,
            this, &MainWindow::slotSelectTrajectory);

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

    cfg.getBool(secName, "ShowServerAddr", tcp_config.show_server_addr);

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
    (void)event;

    map->resize(ui->Map->width(), ui->Map->height());
    map->update();

    bg->resize(ui->Map->width(), ui->Map->height());
    bg->setScale(map->getScale());
    bg->setShift(map->getShift());

    Trajectory* new_nearest_trajectory = map->nearest_trajectory;
    if (route_begin_trajectory && new_nearest_trajectory && (route_dir != 0))
    {
        if (new_nearest_trajectory != bg->nearest_trajectory)
        {
            route_segment_t route = topology->find_route(route_begin_trajectory,
                                                         new_nearest_trajectory,
                                                         route_dir);
            bg->route_trajectories = route.trajectories;
/* Отладка
            QString msg;
            if (bg->route_trajectories.size())
            {
                msg = "Founded route (" + QString::number(bg->route_trajectories.size()) + "):";
                for (auto& traj : bg->route_trajectories)
                {
                    msg += " - ";
                    if (traj)
                        msg += traj->getName();
                    else
                        msg += "null";
                }
            }
            else
            {
                msg = "No route " +
                      route_begin_trajectory->getName() +
                      " - " +
                      new_nearest_trajectory->getName();
            }
            ui->ptLog->appendPlainText(msg);
*/
        }
    }
    else
    {
        bg->route_trajectories.clear();
    }
    bg->nearest_trajectory = map->nearest_trajectory;
    bg->route_begin_trajectory = route_begin_trajectory;

    bg->update();
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

    if ( (topology->getTrajectoriesList() == nullptr) || (topology->getConnectorsList() == nullptr) )
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

    for (auto tl : map->traj_labels)
    {
        delete tl;
    }
    map->traj_labels.clear();

    for (auto traj : *topology->getTrajectoriesList())
    {
        QLabel *traj_label = new QLabel(map);
        traj_label->setText(traj->getName());
        traj_label->setVisible(false);
        map->traj_labels.insert(traj->getName(), traj_label);
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

    if (signals_data->route_signals.size() != 0)
    {
        ui->ptLog->appendPlainText(QString(tr("Loaded %1 route signals")).arg(signals_data->route_signals.size()));
    }
    else
    {
        ui->ptLog->appendPlainText(QString(tr("Warning: no route signals data")));
    }

    if (signals_data->exit_signals.size() != 0)
    {
        ui->ptLog->appendPlainText(QString(tr("Loaded %1 exit signals")).arg(signals_data->exit_signals.size()));
    }
    else
    {
        ui->ptLog->appendPlainText(QString(tr("Warning: no exit signals data")));
    }

    if (signals_data->shunt_signals.size() != 0)
    {
        ui->ptLog->appendPlainText(QString(tr("Loaded %1 shunt signals")).arg(signals_data->shunt_signals.size()));
    }
    else
    {
        ui->ptLog->appendPlainText(QString(tr("Warning: no shunt signals data")));
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

    auto configure_signal_label = [](Signal* sig, Topology* top, MapWidget* map) -> SignalLabel*
    {
        Connector *conn = top->getConnectorsList()->value(sig->getConnectorName(), nullptr);

        if (conn == nullptr)
        {
            return nullptr;
        }

        sig->setConnector(conn);

        SignalLabel* signal_label = new SignalLabel(map);
        signal_label->signal = sig;
        signal_label->setText(sig->getLetter());

        if (sig->getDirection() == 1)
        {
            conn->setSignalFwd(sig);
            map->signal_labels_fwd.insert(conn->getName(), signal_label);
        }

        if (sig->getDirection() == -1)
        {
            conn->setSignalBwd(sig);
            map->signal_labels_bwd.insert(conn->getName(), signal_label);
        }

        return signal_label;
    };

    for (auto& sig : signals_data->line_signals)
    {
        configure_signal_label(sig, topology, map);
    }

    for (auto& sig : signals_data->enter_signals)
    {
        SignalLabel* sl = configure_signal_label(sig, topology, map);
        sl->need_train = true;
        sl->need_call = true;
        connect(sl, &SignalLabel::popUpMenu, this, &MainWindow::slotSignalControlMenu);
    }

    for (auto& sig : signals_data->route_signals)
    {
        SignalLabel* sl = configure_signal_label(sig, topology, map);
        sl->need_train = true;
        sl->need_shunting = true;
        sl->need_call = true;
        connect(sl, &SignalLabel::popUpMenu, this, &MainWindow::slotSignalControlMenu);
    }

    for (auto& sig : signals_data->exit_signals)
    {
        SignalLabel* sl = configure_signal_label(sig, topology, map);
        sl->need_train = true;
        sl->need_shunting = true;
        sl->need_call = true;
        connect(sl, &SignalLabel::popUpMenu, this, &MainWindow::slotSignalControlMenu);
    }

    for (auto& sig : signals_data->shunt_signals)
    {
        SignalLabel* sl = configure_signal_label(sig, topology, map);
        sl->need_shunting = true;
        connect(sl, &SignalLabel::popUpMenu, this, &MainWindow::slotSignalControlMenu);
    }

    // Запрос серверу на регулярное обновление игроков
    tcp_client->sendRequest(STYPE_REQUEST_PLAYERS_INFO,
                            static_cast<double>(players_update_interval) / 1000.0);

    // Запрос серверу на обновление информации о поездах
    tcp_client->sendRequest(STYPE_REQUEST_TRAINS_UPDATE);

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

    // Дата-время сервера в статусной строке внизу
    ui->statusbar->showMessage(train_data.sim_time.getString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotNearestTrajectoryMenu(Trajectory *nearest_traj)
{
    if (route_begin_trajectory)
    {
        // Если уже строим маршрут, то заканчиваем строить маршрут
        slotSelectTrajectory(nearest_traj);
        return;
    }

    if (nearest_traj == nullptr)
    {
        return;
    }

    QMenu* menu = new QMenu(this);

    if (!nearest_traj->isInRoute())
    {
        QAction* route_from_traj_fwd = new QAction(tr("Build route to forward direction"), this);
        menu->addAction(route_from_traj_fwd);
        connect(route_from_traj_fwd, &QAction::triggered, this, [this, nearest_traj]{
            route_begin_trajectory = nearest_traj;
            route_dir = 1;
        });

        QAction* route_from_traj_bwd = new QAction(tr("Build route to backward direction"), this);
        menu->addAction(route_from_traj_bwd);
        connect(route_from_traj_bwd, &QAction::triggered, this, [this, nearest_traj]{
            route_begin_trajectory = nearest_traj;
            route_dir = -1;
        });
    }

    QAction* close_menu = new QAction(tr("Close menu"), this);
    close_menu->setShortcut(QKeySequence(QKeySequence::Cancel));
    menu->addAction(close_menu);

    menu->exec(QCursor::pos());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSelectTrajectory(Trajectory *nearest_traj)
{
    if ((nearest_traj == nullptr) || (route_begin_trajectory == nullptr) || (route_dir == 0))
    {
        route_begin_trajectory = nullptr;
        route_dir = 0;
        return;
    }

    Trajectory* start_traj = route_begin_trajectory;
    Trajectory* target_traj = nearest_traj;
    std::int8_t dir = (route_dir < 0) ? -1 : 1;

    route_begin_trajectory = nullptr;
    route_dir = 0;

    route_segment_t route = topology->find_route(start_traj,
                                                 target_traj,
                                                 dir);

    QMenu* menu = new QMenu(this);

    if (route.trajectories.size() < 2)
    {
        QAction* continue_search = new QAction(tr("Continue route search"), this);
        menu->addAction(continue_search);
        connect(continue_search, &QAction::triggered, this, [this, start_traj, dir]{
            route_begin_trajectory = start_traj;
            route_dir = dir;
        });
    }
    else
    {
        // Указатель на сетевого клиента, который отправит команду построить маршрут
        TcpClient* tc = tcp_client;

        QAction* continue_search = new QAction(tr("Continue route search"), this);
        menu->addAction(continue_search);
        connect(continue_search, &QAction::triggered, this, [this, start_traj, dir]{
            route_begin_trajectory = start_traj;
            route_dir = dir;
        });

        QAction* build_route = new QAction(tr("Set switches for route"), this);
        menu->addAction(build_route);
        connect(build_route, &QAction::triggered, this, [tc, start_traj, target_traj, dir]{
            route_command_t sc = {start_traj->getName(), target_traj->getName(), dir};
            tc->sendBuildRouteCommand(sc.serialize());
        });

        QAction* train_route = new QAction(tr("Set switches and train signals for route"), this);
        menu->addAction(train_route);
        connect(train_route, &QAction::triggered, this, [tc, start_traj, target_traj, dir]{
            route_command_t sc = {start_traj->getName(), target_traj->getName(), dir};
            tc->sendTrainRouteCommand(sc.serialize());
        });

        QAction* shunting_route = new QAction(tr("Set switches and shunting signals for route"), this);
        menu->addAction(shunting_route);
        connect(shunting_route, &QAction::triggered, this, [tc, start_traj, target_traj, dir]{
            route_command_t sc = {start_traj->getName(), target_traj->getName(), dir};
            tc->sendBuildRouteCommand(sc.serialize());
        });
    }

    QAction* close_menu = new QAction(tr("Close menu"), this);
    close_menu->setShortcut(QKeySequence(QKeySequence::Cancel));
    menu->addAction(close_menu);

    menu->exec(QCursor::pos());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSwitchConnectorMenu()
{
    // Создаём меню для текстовой метки стрелочного перевода
    SwitchLabel* sw_label = dynamic_cast<SwitchLabel *>(sender());

    Switch* sw = dynamic_cast<Switch *>(sw_label->conn);
    int state_fwd = sw->getStateFwd();
    int state_bwd = sw->getStateBwd();

    // Если это не стрелка, меню не нужно
    if ((state_fwd == 0) && (state_bwd == 0))
        return;

    // Указатель на сетевого клиента, который отправит команду переключить стрелку
    TcpClient* tc = tcp_client;

    // Создаём меню, передаём в текстовую метку указатель для интерактивной подсветки
    QMenu* menu = new QMenu(this);
    sw_label->menu = menu;

    if (state_fwd != 0)
    {
        // Создаём пункт меню для переключения стрелки в направлении вперёд
        QAction* action_switch_fwd = new QAction(tr("Switch forward"), this);
        action_switch_fwd->setEnabled((sw->getStateFwd() == Switch::STATE_MINUS) ||
                                      (sw->getStateFwd() == Switch::STATE_PLUS));
        menu->addAction(action_switch_fwd);

        sw_label->action_switch_fwd = action_switch_fwd;
        connect(action_switch_fwd, &QAction::triggered, sw_label, &SwitchLabel::resetMenu);

        // Создаём сетевой пакет с командой переключения стрелки в направлении вперёд
        connect(action_switch_fwd, &QAction::triggered, this, [sw, tc]{
            std::int8_t ref_state = (sw->getStateFwd() < 0) ? 1 : -1;
            switch_command_t sc = {sw->getName(), 1, ref_state};
            tc->sendSwitchCommand(sc.serialize());
        });
    }

    if (state_bwd != 0)
    {
        // Создаём пункт меню для переключения стрелки в направлении назад
        QAction* action_switch_bwd = new QAction(tr("Switch backward"), this);
        action_switch_bwd->setEnabled((sw->getStateBwd() == Switch::STATE_MINUS) ||
                                      (sw->getStateBwd() == Switch::STATE_PLUS));
        menu->addAction(action_switch_bwd);

        sw_label->action_switch_bwd = action_switch_bwd;
        connect(action_switch_bwd, &QAction::triggered, sw_label, &SwitchLabel::resetMenu);

        // Создаём сетевой пакет с командой переключения стрелки в направлении назад
        connect(action_switch_bwd, &QAction::triggered, this, [sw, tc]{
            std::int8_t ref_state = (sw->getStateBwd() < 0) ? 1 : -1;
            switch_command_t sc = {sw->getName(), -1, ref_state};
            tc->sendSwitchCommand(sc.serialize());
        });
    }

    menu->exec(QCursor::pos());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSignalControlMenu()
{
    // Создаём меню для текстовой метки с литером светофора
    SignalLabel* signal_label = dynamic_cast<SignalLabel *>(sender());

    if (!(signal_label->need_train || signal_label->need_shunting || signal_label->need_call))
    {
        return;
    }

    Signal* sig = signal_label->signal;

    // Указатель на сетевого клиента, который отправит команду светофору
    TcpClient* tc = tcp_client;

    // Создаём меню, передаём в текстовую метку указатель для интерактивной подсветки
    QMenu* menu = new QMenu(this);
    signal_label->menu = menu;

    if (signal_label->need_train)
    {
        // Создаём пункт меню для открытия сигнала поездным маршрутом
        QAction *open_train = new QAction(tr("Open for train"), this);
        menu->addAction(open_train);

        signal_label->action_open_train = open_train;
        connect(open_train, &QAction::triggered, signal_label, &SignalLabel::resetMenu);

        // Создаём сетевой пакет с командой открытия сигнала
        connect(open_train, &QAction::triggered, this, [tc, sig]{
            std::int8_t dir = sig->getDirection();
            signal_command_t sc = {sig->getConnector()->getName(), dir,
                                   true, false, false, false};
            tc->sendSignalCommand(sc.serialize());
        });
    }

    if (signal_label->need_shunting)
    {
        // Создаём пункт меню для открытия сигнала маневровым маршрутом
        QAction *open_train = new QAction(tr("Open for shunting"), this);
        menu->addAction(open_train);

        signal_label->action_open_train = open_train;
        connect(open_train, &QAction::triggered, signal_label, &SignalLabel::resetMenu);

        // Создаём сетевой пакет с командой открытия сигнала
        connect(open_train, &QAction::triggered, this, [tc, sig]{
            std::int8_t dir = sig->getDirection();
            signal_command_t sc = {sig->getConnector()->getName(), dir,
                                   false, true, false, false};
            tc->sendSignalCommand(sc.serialize());
        });
    }

    if (signal_label->need_call)
    {
        // Создаём пункт меню для открытия пригласительного сигнала
        QAction *open_train = new QAction(tr("Open call signal"), this);
        menu->addAction(open_train);

        signal_label->action_open_train = open_train;
        connect(open_train, &QAction::triggered, signal_label, &SignalLabel::resetMenu);

        // Создаём сетевой пакет с командой открытия сигнала
        connect(open_train, &QAction::triggered, this, [tc, sig]{
            std::int8_t dir = sig->getDirection();
            signal_command_t sc = {sig->getConnector()->getName(), dir,
                                   false, false, true, false};
            tc->sendSignalCommand(sc.serialize());
        });
    }

    // Создаём пункт меню для закрытия сигнала
    QAction *close = new QAction(tr("Close"), this);
    menu->addAction(close);

    signal_label->action_close = close;
    connect(close, &QAction::triggered, signal_label, &SignalLabel::resetMenu);

    // Создаём сетевой пакет с командой закрытия сигнала
    connect(close, &QAction::triggered, this, [tc, sig]{
        std::int8_t dir = sig->getDirection();
        signal_command_t sc = {sig->getConnector()->getName(), dir,
                               false, false, false, true};
        tc->sendSignalCommand(sc.serialize());
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

    Switch *sw = dynamic_cast<Switch *>(topology->getConnectorsList()->value(switch_state.name, nullptr));

    if (sw == nullptr)
    {
        return;
    }

    sw->setStateFwd(Switch::State(switch_state.state_fwd));
    sw->setStateBwd(Switch::State(switch_state.state_bwd));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetTrajBusyState(QByteArray &busy_data)
{
    traj_busy_state_t busy_state;
    busy_state.deserialize(busy_data);

    Trajectory *traj = (topology->getTrajectoriesList()->value(busy_state.name, nullptr));

    if (traj == nullptr)
    {
        return;
    }

    traj->setBusyState(busy_state.is_busy);
    traj->setInRoute(busy_state.in_route);
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

    Connector *conn = topology->getConnectorsList()->value(conn_name, nullptr);

    if (conn == nullptr)
    {
        return;
    }

    if (signal_dir == 1)
    {
        Signal* sig = conn->getSignalFwd();
        if (sig)
        {
            sig->deserialize(signal_data);
        }
    }

    if (signal_dir == -1)
    {
        Signal* sig = conn->getSignalBwd();
        if (sig)
        {
            sig->deserialize(signal_data);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotRecvLogMessage(QString msg)
{
    ui->ptLog->appendPlainText(msg);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSetShowTrajStatus(bool is_show)
{
    map->showTrajNames(is_show);

    if (is_show)
        slotRecvLogMessage(tr("Showed traj names"));
    else
        slotRecvLogMessage(tr("Hided traj names"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotGetTrainsInfo(QByteArray &data)
{
    simulator_trains_update_t update_trains;
    update_trains.deserialize(data);

    for (auto tl : map->train_labels)
    {
        delete tl;
    }

    map->train_labels.clear();

    for (size_t i = 0; i < update_trains.trains.size(); ++i)
    {
        TrainLabel *train_label = new TrainLabel(map);
        train_label->setAlignment(Qt::AlignHCenter);
        train_label->setStyleSheet("color: white;");

        QString train_name = update_trains.trains[i].train_name;

        if (!train_name.isEmpty())
            train_label->setText(train_name);
        else
            train_label->setText("0000");

        train_label->first_vehicle_idx = update_trains.trains[i].first_vehicle_id;
        train_label->train_idx = i;

        connect(train_label, &TrainLabel::popUpMenu, this, &MainWindow::slotRenameTrainMenu);

        map->train_labels.push_back(train_label);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotRenameTrainMenu()
{
    TrainLabel *train_label = dynamic_cast<TrainLabel *>(sender());

    QMenu *menu = new QMenu(this);

    QAction *rename = new QAction(tr("Rename"), this);
    menu->addAction(rename);

    connect(rename, &QAction::triggered, this, [this, train_label]{

        bool ok = false;
        QString new_name = QInputDialog::getText(
            map,                        // родительский виджет (можно указать this, если вызывается из QWidget)
            tr("Enter train name"),           // заголовок окна
            "",                               // метка (label)
            QLineEdit::Normal,                // режим ввода (Normal, Password и т.д.)
            "",                               // начальное значение
            &ok                              // указатель на bool: true — если нажата OK, false — если Cancel
            );

        if (ok && !new_name.isEmpty()) {
            this->tcp_client->sendNewTrainName(train_label->train_idx, new_name);
        }
    });

    menu->exec(QCursor::pos());
}
