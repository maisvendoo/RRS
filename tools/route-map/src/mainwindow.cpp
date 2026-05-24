#include    <physics.h>
#include    <signal-command.h>
#include    <mainwindow.h>
#include    <rail-signal.h>
#include    <ui_mainwindow.h>

#include    <CfgReader.h>
#include    <QPainter>
#include    <QMenu>
#include    <switch.h>
#include    <switch-state.h>
#include    <QInputDialog>
#include    <QClipboard>
#include    <styles.h>
#include    <filesystem.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainWindow::MainWindow(route_map_command_line_t &cmd_line, QWidget *parent): QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setWindowTitle(tr("route-map"));

    ui->setupUi(this);

    loadSettingsGUI();

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

    connect(ui->actionShowConnName, &QAction::triggered,
            this, &MainWindow::slotSetShowConnStatus);

    connect(tcp_client, &TcpClient::setTrainInfo,
            this, &MainWindow::slotGetTrainsInfo);

    for (auto action : ui->mSimSpeed->actions())
    {
        connect(action, &QAction::triggered, this, &MainWindow::slotSetSimSpeed);
    }

    connect(ui->actionShow_trajectories_tooltips, &QAction::triggered, this, &MainWindow::slotSetShowTrajTooltip);

    connect(ui->actionShow_server_time, &QAction::triggered, this, &MainWindow::slotShowSimTime);

    bg = new BackGroundWidget(ui->Map);

    map = new MapWidget(ui->Map);
    map->stations = topology->getStationsList();
    map->traj_list = topology->getTrajectoriesList();
    map->conn_list = topology->getConnectorsList();
    map->signals_data = signals_data;
    map->train_data = &train_data;
    map->vehicles_half_length = &vehicles_half_length;
    map->players_data = &players_data;

    connect(map, &MapWidget::sigOpenSignalMenu,
            this, &MainWindow::slotSignalControlMenu);

    connect(map, &MapWidget::sigOpenSwitchMenu,
            this, &MainWindow::slotNearestSwitchMenu);

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
void MainWindow::loadSettingsGUI()
{
    FileSystem &fs = FileSystem::getInstance();
    std::string cfg_dir = fs.getConfigDir();
    std::string cfg_path = fs.combinePath(cfg_dir, "gui-settings.xml");

    CfgReader cfg;

    if ( cfg.load(QString(cfg_path.c_str())) )
    {
        QString secName = "GUISettings";
        QString theme_name = "";

        if (!cfg.getString(secName, "Theme", theme_name))
        {
            theme_name = "dark-jedy";
        }

        std::string theme_dir = fs.getThemeDir();
        std::string theme_path = fs.combinePath(theme_dir, theme_name.toStdString() + ".qss");
        QString style_sheet = readStyleSheet(QString(theme_path.c_str()));

        this->setStyleSheet(style_sheet);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::updateSimSpeedMenu(int speed_factor)
{
    for (auto *action : ui->mSimSpeed->actions())
    {
        int idx = ui->mSimSpeed->actions().indexOf(action);

        if (idx == speed_factor)
        {
            action->setChecked(true);
        }
        else
        {
            action->setChecked(false);
        }
    }
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
    double tmp_value = 0.0;
    cfg.getDouble(secName, "SwitchLength", tmp_value);
    if (tmp_value > Physics::ZERO)
    {
        map->setSwitchLength(tmp_value);
        bg->setSwitchLength(tmp_value);
    }

    tmp_value = 0.0;
    cfg.getDouble(secName, "SignalRadius", tmp_value);
    if (tmp_value > Physics::ZERO)
    {
        map->setSignalRadius(tmp_value);
        bg->setSignalRadius(tmp_value);
    }

    tmp_value = 0.0;
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

    if (!is_menu_shows)
    {
        Trajectory* new_nearest_trajectory = nullptr;
        Signal* new_nearest_signal = map->nearest_signal;
        if (new_nearest_signal)
        {
            dir_t dir = static_cast<dir_t>(-1 * new_nearest_signal->getDirection());
            new_nearest_trajectory = new_nearest_signal->getConnector()->getNextTraj(dir);

            bg->nearest_signal = new_nearest_signal;
            bg->nearest_signal_coord = map->nearest_signal_coord;

            bg->nearest_switch = nullptr;
            bg->nearest_switch_dir = 0;
        }
        else
        {
            bg->nearest_signal = nullptr;
            new_nearest_trajectory = map->nearest_trajectory;

            Switch* new_nearest_switch = map->nearest_switch;
            if (new_nearest_switch && (map->nearest_switch_dir != 0))
            {
                bg->nearest_switch = new_nearest_switch;
                bg->nearest_switch_dir = map->nearest_switch_dir;
            }
            else
            {
                bg->nearest_switch = nullptr;
                bg->nearest_switch_dir = 0;
            }
        }

        if (route_begin_trajectory && new_nearest_trajectory && (route_dir != 0))
        {
            if (new_nearest_trajectory != bg->nearest_trajectory)
            {
                route_segment_t route = topology->find_route(route_begin_trajectory,
                                                             new_nearest_trajectory,
                                                             route_dir);
                bg->route_trajectories = route.trajectories;
            }
        }
        else
        {
            bg->route_trajectories.clear();
        }
        bg->nearest_trajectory = new_nearest_trajectory;
        bg->route_begin_trajectory = route_begin_trajectory;
    }

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
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (tcp_client != nullptr)
    {
        disconnect(tcp_client, nullptr, this, nullptr);
        tcp_client->slotDisconnect();
        tcp_client->deleteLater();
    }

    QMainWindow::closeEvent(event);
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
    map->calcCwitchCoords();

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
        QLabel *sw_label = new QLabel(map);
        sw_label->setText(conn->getName());
        sw_label->setVisible(false);

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

    for (auto sl : map->signal_labels)
    {
        delete sl;
    }
    map->signal_labels.clear();

    auto configure_signal_label = [](Signal* sig, Topology* top, MapWidget* map) -> SignalLabel*
    {
        Switch* conn = top->getConnectorsList()->value(sig->getConnectorName(), nullptr);

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
            map->signal_labels.insert(sig, signal_label);
        }

        if (sig->getDirection() == -1)
        {
            conn->setSignalBwd(sig);
            map->signal_labels.insert(sig, signal_label);
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

    QString formatted_time = train_data.sim_time.time.getString();

    updateSimSpeedMenu(train_data.speed_factor);

    // Убедиться что формат "HH:MM:SS" (8 символов всегда)
    if (formatted_time.length() < 8)
    {
        formatted_time = formatted_time.rightJustified(8, '0');
    }
    map->setSimTime(formatted_time);

    // Дата-время сервера в статусной строке внизу
    ui->statusbar->showMessage(train_data.sim_time.getString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotNearestSwitchMenu(Switch* nearest_conn, std::int8_t nearest_switch_dir)
{
    if (route_begin_trajectory && map->nearest_trajectory)
    {
        // Если уже строим маршрут, то заканчиваем строить маршрут
        slotSelectTrajectory(map->nearest_trajectory);
        return;
    }

    // Создаём меню для стрелочного перевода
    Switch* sw = dynamic_cast<Switch *>(nearest_conn);
    if ((sw == nullptr) || (nearest_switch_dir == 0))
    {
        return;
    }

    // Если это не стрелка, меню не нужно
    Switch_state_t state = (nearest_switch_dir > 0) ? sw->getStateFwd() : sw->getStateBwd();
    if (   (state == NO_POSSIBLE_DIRECTION)
        || (state == ONLY_MINUS)
        || (state == ONLY_PLUS))
    {
        return;
    }

    // Указатель на сетевого клиента, который отправит команду переключить стрелку
    TcpClient* tc = tcp_client;

    // Создаём меню
    QMenu* menu = new QMenu(this);

    // Создаём пункт меню для переключения стрелки
    QAction* action_switch = new QAction(tr("Switch"), this);
    action_switch->setEnabled((state == STATE_MINUS) ||
                              (state == STATE_PLUS));
    menu->addAction(action_switch);

    // Сохраняем в карте указатель для интерактивных действий по наведению курсора
    map->switch_menu = {menu, action_switch, nearest_conn, nearest_switch_dir};
    // И подключаем слот для сброса сохранённого указателя
    connect(action_switch, &QAction::triggered, map, &MapWidget::resetSwitchMenu);

    // Создаём сетевой пакет с командой переключения стрелки
    std::int8_t switch_dir = nearest_switch_dir;
    connect(action_switch, &QAction::triggered, this, [sw, switch_dir, tc]{
        Switch_state_t cur_state = (switch_dir > 0) ? sw->getStateFwd() : sw->getStateBwd();
        switch_command_t sc;
        sc.conn_name = sw->getName();
        sc.switch_direction = switch_dir;
        sc.switch_ref_state = (cur_state < 0) ? STATE_PLUS : STATE_MINUS;
        tc->sendSwitchCommand(sc.serialize());
    });

    // Сброс заморозки выделения ближайших объектов
    connect(menu, &QMenu::aboutToHide, this, &MainWindow::slotMenuHide);

    // Заморозка выделения ближайших объектов и показ меню
    is_menu_shows = true;
    menu->exec(QCursor::pos());
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
//    bool no_need_menu = true;

    // Проверяем, есть ли траектория вперёд
    dir_t dir = FWD;
    if (Switch* conn_fwd = nearest_traj->getNextSwitch(dir))
    {
        if (Trajectory* next_fwd = conn_fwd->getNextTraj(dir))
        {
//            no_need_menu = false;
            // Создаём пункт меню для поиска маршрута в направлении вперёд
            QAction* route_from_traj_fwd = new QAction(tr("Build route to forward direction"), this);
            menu->addAction(route_from_traj_fwd);
            connect(route_from_traj_fwd, &QAction::triggered, this, [this, nearest_traj]{
                route_begin_trajectory = nearest_traj;
                route_dir = 1;
            });
        }
    }

    // Проверяем, есть ли траектория назад
    dir = BWD;
    if (Switch* conn_bwd = nearest_traj->getNextSwitch(dir))
    {
        if (Trajectory* next_bwd = conn_bwd->getNextTraj(dir))
        {
//            no_need_menu = false;
            // Создаём пункт меню для поиска маршрута в направлении назад
            QAction* route_from_traj_bwd = new QAction(tr("Build route to backward direction"), this);
            menu->addAction(route_from_traj_bwd);
            connect(route_from_traj_bwd, &QAction::triggered, this, [this, nearest_traj]{
                route_begin_trajectory = nearest_traj;
                route_dir = -1;
            });
        }
    }

    // Копирование имени траектории в буффер обмена (для сценаристов)
//    no_need_menu = false;
    QAction *copy_name_to_clipboard = new QAction(tr("Copy trajectory name to clipboard"), this);
    menu->addAction(copy_name_to_clipboard);
    connect(copy_name_to_clipboard, &QAction::triggered, this, [this, nearest_traj]{
        QApplication::clipboard()->setText(nearest_traj->getName(), QClipboard::Clipboard);
    });
/*
    if (no_need_menu)
    {
        delete menu;
        return;
    }
*/
    connect(menu, &QMenu::aboutToHide, this, &MainWindow::slotMenuHide);
    is_menu_shows = true;
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
        // Создаём пункт меню для продолжения поиска маршрута
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

        // Создаём пункт меню для продолжения поиска маршрута
        QAction* continue_search = new QAction(tr("Continue route search"), this);
        menu->addAction(continue_search);
        connect(continue_search, &QAction::triggered, this, [this, start_traj, dir]{
            route_begin_trajectory = start_traj;
            route_dir = dir;
        });

        // Создаём пункт меню для установки стрелок по маршруту
        QAction* build_route = new QAction(tr("Set switches for route"), this);
        menu->addAction(build_route);
        connect(build_route, &QAction::triggered, this, [tc, start_traj, target_traj, dir]{
            route_command_t sc = {start_traj->getName(), target_traj->getName(), dir};
            tc->sendBuildRouteCommand(sc.serialize());
        });

        // Создаём пункт меню для установки стрелок и открытия поездных светофоров по маршруту
        QAction* train_route = new QAction(tr("Set switches and train signals for route"), this);
        menu->addAction(train_route);
        connect(train_route, &QAction::triggered, this, [tc, start_traj, target_traj, dir]{
            route_command_t sc = {start_traj->getName(), target_traj->getName(), dir};
            tc->sendTrainRouteCommand(sc.serialize());
        });

        // Создаём пункт меню для установки стрелок и открытия маневровых светофоров по маршруту
        QAction* shunting_route = new QAction(tr("Set switches and shunting signals for route"), this);
        menu->addAction(shunting_route);
        connect(shunting_route, &QAction::triggered, this, [tc, start_traj, target_traj, dir]{
            route_command_t sc = {start_traj->getName(), target_traj->getName(), dir};
            tc->sendShuntingRouteCommand(sc.serialize());
        });
    }

    connect(menu, &QMenu::aboutToHide, this, &MainWindow::slotMenuHide);
    is_menu_shows = true;
    menu->exec(QCursor::pos());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSignalControlMenu(Signal* sig)
{
    if (route_begin_trajectory)
    {
        // Если уже строим маршрут, то заканчиваем строить маршрут до траектории перед светофором
        dir_t dir = static_cast<dir_t>(-1 * sig->getDirection());
        Trajectory* target_traj = sig->getConnector()->getNextTraj(dir);
        slotSelectTrajectory(target_traj);
        return;
    }

    // Создаём меню для текстовой метки с литером светофора
    SignalLabel* signal_label = map->signal_labels.value(sig, nullptr);
    if (!signal_label)
    {
        return;
    }

    // Светофорам без управления маршрутами меню не нужно
    if (!(signal_label->need_train || signal_label->need_shunting || signal_label->need_call))
    {
        return;
    }

    // Указатель на сетевого клиента, который отправит команду светофору
    TcpClient* tc = tcp_client;

    // Создаём меню, передаём указатель на меню
    // в текстовую метку для интерактивных действий по наведению курсора (пока не используется)
    QMenu* menu = new QMenu(this);
    signal_label->menu = menu;

    // Создаём пункт меню для построения маршрута от светофора
    QAction *build_route = new QAction(tr("Build route..."), this);
    menu->addAction(build_route);

    signal_label->action_build_route = build_route;
    connect(build_route, &QAction::triggered, signal_label, &SignalLabel::resetMenu);

    // Создаём команду маршрута от траектории перед светофором в направлении этого светофора
    connect(build_route, &QAction::triggered, this, [this, sig]{
        dir_t dir = static_cast<dir_t>(-1 * sig->getDirection());
        route_begin_trajectory = sig->getConnector()->getNextTraj(dir);
        route_dir = -1 * dir;
    });

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

    connect(menu, &QMenu::aboutToHide, this, &MainWindow::slotMenuHide);
    is_menu_shows = true;
    menu->exec(QCursor::pos());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotMenuHide()
{
    is_menu_shows = false;
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

    sw->setStateFwd(Switch_state_t(switch_state.state_fwd));
    sw->setStateBwd(Switch_state_t(switch_state.state_bwd));
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
    int8_t signal_dir = 0;

    stream >> conn_name;
    stream >> signal_dir;

    if (conn_name.isEmpty())
    {
        return;
    }

    Switch* conn = topology->getConnectorsList()->value(conn_name, nullptr);

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
    map->show_traj_names = is_show;

    if (is_show)
        slotRecvLogMessage(tr("Showed traj names"));
    else
        slotRecvLogMessage(tr("Hided traj names"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSetShowConnStatus(bool is_show)
{
    map->show_conn_names = is_show;

    if (is_show)
        slotRecvLogMessage(tr("Showed conn names"));
    else
        slotRecvLogMessage(tr("Hided conn names"));
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
    ui->mTrains->clear();

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

        QAction *action_train = new QAction(train_name);
        ui->mTrains->addAction(action_train);

        MapWidget *mw = map;
        int vehicle_idx = update_trains.trains[i].first_vehicle_id;

        connect(action_train, &QAction::triggered, this, [mw, vehicle_idx]{
            mw->slotSetVehicleAtCenter(vehicle_idx);
        });
    }

    if (!update_trains.trains.empty())
    {
        int vehicle_idx = update_trains.trains[0].first_vehicle_id;
        map->slotSetVehicleAtCenter(0);
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSetSimSpeed(bool is_cheked)
{
    for (auto action : ui->mSimSpeed->actions())
    {
        action->setChecked(false);
    }

    QAction *action = dynamic_cast<QAction *>(sender());
    action->setChecked(true);

    int idx = ui->mSimSpeed->actions().indexOf(action);

    if (idx < 0)
    {
        return;
    }

    int speed_factor = 0;

    if (idx == 0)
    {
        speed_factor = 0;
    }
    else
    {
        speed_factor = 1 << (idx - 1);
    }

    tcp_client->sendSimSpeedCommand(speed_factor);

    slotRecvLogMessage(QString(tr("Simulation speed x%1")).arg(speed_factor));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotSetShowTrajTooltip(bool is_show)
{
    map->setShowTrajectoryTooltip(is_show);

    if (is_show)
        slotRecvLogMessage(tr("Enabled trajectory tooltips"));
    else
        slotRecvLogMessage(tr("Disabled trajectory tooltips"));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainWindow::slotShowSimTime(bool is_show)
{
    if (map == nullptr)
    {
        return;
    }

    map->showSimTime(is_show);
}
