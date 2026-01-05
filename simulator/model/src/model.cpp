//------------------------------------------------------------------------------
//
//      Train motion model simulation control
//      (c) maisvendoo, 02/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Train motion model simulation control
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 02/09/2018
 */

#include    "model.h"

#include    <QTime>

#include    <CfgReader.h>
#include    <Journal.h>
#include    <JournalFile.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Model::Model(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Model::~Model()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Model::init(const simulator_command_line_t &command_line)
{
    init_data_t init_data;

    // Load initial data configuration
    loadInitData(init_data);

    init_datas.push_back(init_data);

    // Override init data by command line
    overrideByCommandLine(init_data, command_line);

    // Read solver configuration
    configSolver(init_data.solver_config);

    // Load route topology
    initTopology(init_data);

    // Init scenario's manager
    if (initScenarioManager(init_data, command_line))
    {
        init_datas = scnmgr->init_datas;
        init_data.start_datetime = scnmgr->getStartDateTime();
    }

    // Create all trains
    for (size_t i = 0; i < init_datas.size(); ++i)
    {
        Train *train = addTrain(init_datas[i]);
        if (train)
        {
            trains.push_back(train);

            QThread *thread = new QThread();
            train_threads.push_back(thread);
            train->moveToThread(thread);
            Journal::instance()->info(QString("Created new thread for train at address: 0x%1")
                                          .arg(reinterpret_cast<quint64>(thread), 0, 16));

            connect(this, &Model::step, train, &Train::slotStep);
            connect(train, &Train::stepDone, this, &Model::slotTrainStepDone);
            thread->start();
        }
    }

    initControlPanel("control-panel");

    //initTraffic(init_data);

    start_time = init_data.solver_config.start_time;
    integration_time_interval = init_data.integration_time_interval;
    if (init_data.start_datetime > 0)
    {
        sim_time = simulator_time_t(init_data.start_datetime);
    }
    sim_time.simulation_seconds = start_time;

    Journal::instance()->info("==== Info to server ====");
    simulator_route_info_t route_info = simulator_route_info_t();

    FileSystem &fs = FileSystem::getInstance();
    QString cfg_path = QString(fs.getRouteRootDir().c_str()) +
                       QDir::separator() + init_data.route_dir_name +
                       QDir::separator() + "description.xml";
    CfgReader cfg;
    if (cfg.load(cfg_path))
    {
        cfg.getDouble("Route", "Latitude", route_info.latitude);
        cfg.getDouble("Route", "Longitude", route_info.longitude);
    }

    route_info.route_dir_name = init_data.route_dir_name;
    tcp_server->setRouteInfo(route_info.serialize());
    Journal::instance()->info("Ready route info for server");

    simulator_vehicles_info_t vehicles_info;
    vehicles_info.vehicles.resize(vehicles.size());
    size_t i = 0;
    for (auto it = vehicles.begin(); it != vehicles.end(); ++it)
    {
        vehicles_info.vehicles[i].vehicle_length = (*it)->getLength();
        vehicles_info.vehicles[i].vehicle_config_dir = (*it)->getConfigDir();
        vehicles_info.vehicles[i].vehicle_config_file = (*it)->getConfigName();
        ++i;
    }
    tcp_server->setVehiclesInfo(vehicles_info.serialize());
    Journal::instance()->info("Ready vehicles info for server");

    initTcpServer();

    Journal::instance()->info("Simulator model and server are initialized successfully");

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::start()
{
    if (!isStarted())
    {
        is_simulation_started = true;

        connect(&simTimer, &ElapsedTimer::process, this, &Model::process, Qt::DirectConnection);
        simTimer.setInterval(static_cast<quint64>(integration_time_interval));
        simTimer.start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Model::isStarted() const
{
    return is_simulation_started;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::outMessage(QString msg)
{
    fputs(qPrintable(msg + "\n"), stdout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::deleteFinishedThread()
{
    QThread *thread = dynamic_cast<QThread *>(sender());
    disconnect(thread, &QThread::finished, this, &Model::deleteFinishedThread);
    delete thread;

    Journal::instance()->info(QString("Delete finished thread at address: %1")
                                  .arg(reinterpret_cast<quint64>(thread), 0, 16));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::controlProcess()
{
    if (vehicle_controlled_by_panel && control_panel)
    {
        emit sendSignalsToControlPanel(vehicle_controlled_by_panel->getFeedBackSignals());
        control_panel->process();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::receiveSignalsFromControlPanel(const control_signals_t &control_signals)
{
    if (vehicle_controlled_by_panel)
        vehicle_controlled_by_panel->setControlSignals(control_signals);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::findNearestVehicles()
{
    struct founded_distance
    {
        size_t train_idx;   ///< Индекс поезда в симуляции
        bool from_head;     ///< Признак сближения с другим поездом головой или хвостом
        double distance;    ///< Дистанция между поездами
    };

    // Массив для всех найденных пар близкорасположенных поездов
    QMap<size_t, founded_distance> nearest_trains;
    std::vector<size_t> trains_idx_to_delete;

    for (size_t train_idx = 0; train_idx < trains.size(); ++train_idx)
    {
        Train* train = trains[train_idx];
        if (train == nullptr)
        {
            continue;
        }

        int train_dir = train->getDirection();

        // От каждого поезда ищем вперёд и назад по топологии
        for (int dir_it : {1, -1})
        {
            // Индекс крайней ПЕ в поезде, от которой начинаем поиск
            int idx = (train_dir == dir_it) ?
                              train->getFirstVehicle()->getModelIndex() :
                              train->getLastVehicle()->getModelIndex();

            // Ищем другую ПЕ в пределах 10 метров, и дистанцию до неё в данный момент
            double current_distance = 0.0;
            int nearest_idx = topology->getVehicleController(idx)->getNearestVehicle(
                current_distance, DISTANCE_TO_COUPLE_TRAINS, dir_it);

            // Если ничего не нашли - дальше делать нечего
            if (nearest_idx == -1)
            {
                train->setDistanceToEndOfTrajectory(dir_it, current_distance);
                continue;
            }

            train->setDistanceToEndOfTrajectory(dir_it, DISTANCE_TO_COUPLE_TRAINS);

            // Создаём число из индексов найденной пары ПЕ, в порядке возрастания
            size_t idx_pair = (idx < nearest_idx) ?
                                  MAX_NUM_VEHICLES * idx + nearest_idx :
                                  MAX_NUM_VEHICLES * nearest_idx + idx;

            // Поскольку предполагается, что поиск найдёт каждую пару ПЕ дважды,
            // то проверяем что эта пара уже найдена в предыдущих поездах
            if (nearest_trains.contains(idx_pair))
            {
                // Найденную дважды пару ПЕ соединяем в один поезд
                founded_distance fd = nearest_trains.value(idx_pair);

                // Но проверяем, если поезд замкнулся сам на себя - игнорируем
                if (fd.train_idx == train_idx)
                {
                    nearest_trains.remove(idx_pair);
                    break;
                }

                Journal::instance()->info(QString("t = %1s Founded vehicles #%2 and #%3 at distance %4 (%5) m")
                                              .arg(sim_time.simulation_seconds, 10, 'f', 3)
                                              .arg(idx)
                                              .arg(nearest_idx)
                                              .arg(fd.distance, 7, 'f', 3)
                                              .arg(current_distance, 7, 'f', 3));
                Journal::instance()->info(QString("t = %1s Connect trains #%2 (from %3) and #%4 (from %5)")
                                              .arg(sim_time.simulation_seconds, 10, 'f', 3)
                                              .arg(fd.train_idx)
                                              .arg(fd.from_head ? "head" : "tail")
                                              .arg(train_idx)
                                              .arg((train_dir == dir_it) ? "head" : "tail"));
                trains[fd.train_idx]->couple(current_distance, fd.from_head, (train_dir == dir_it), train);

                // Поезд прицеплен и больше не нужен, запоминаем его чтобы удалить
                trains_idx_to_delete.push_back(train_idx);

                // Найденная пара ПЕ тоже не нужна
                nearest_trains.remove(idx_pair);
                break;
            }
            else
            {
                // Сохраняем найденную пару ПЕ
                founded_distance fd;
                fd.train_idx = train_idx;
                fd.from_head = (train_dir == dir_it);
                fd.distance = current_distance;
                nearest_trains.insert(idx_pair, fd);
            }
        }
    }

    if (trains_idx_to_delete.empty())
        return;

    // Сортируем индексы поездов по убыванию
    std::sort(trains_idx_to_delete.begin(), trains_idx_to_delete.end(), std::greater<size_t>());

    // Удаляем прицепленные поезда
    for (auto train_idx : trains_idx_to_delete)
    {
        disconnect(this, &Model::step, trains[train_idx], &Train::slotStep);
        disconnect(trains[train_idx], &Train::stepDone, this, &Model::slotTrainStepDone);

        trains[train_idx]->moveToThread(this->thread());
        delete trains[train_idx];
        trains.erase(trains.begin() + train_idx);

        connect(train_threads[train_idx], &QThread::finished, this, &Model::deleteFinishedThread);
        train_threads[train_idx]->quit();
        Journal::instance()->info(QString("Delete train #%1 and quit its thread at address: %2")
                                      .arg(train_idx, 3)
                                      .arg(reinterpret_cast<quint64>(train_threads[train_idx]), 0, 16));

        train_threads.erase(train_threads.begin() + train_idx);
    }

    // Назначаем новые порядковые индексы поездам после уменьшения массива
    for (size_t train_idx = trains_idx_to_delete.back(); train_idx < trains.size(); ++train_idx)
    {
        Journal::instance()->info(QString("Train #%1 now #%2")
                                      .arg(trains[train_idx]->getTrainIndex(), 3)
                                      .arg(train_idx, 3));
        trains[train_idx]->setTrainIndex(train_idx);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::findFarthestVehicles()
{
    for (auto train : trains)
    {
        Train *uncoupled_train = train->uncouple(DISTANCE_TO_UNCOUPLE_TRAINS);
        if (uncoupled_train != nullptr)
        {
            Journal::instance()->info(QString("Uncoupled new train #%1 ")
                                          .arg(trains.size(), 3));
            uncoupled_train->setTrainIndex(trains.size());
            trains.push_back(uncoupled_train);

            QThread *thread = new QThread();
            train_threads.push_back(thread);
            uncoupled_train->moveToThread(thread);
            Journal::instance()->info(QString("Created new thread for train at address: 0x%1")
                                          .arg(reinterpret_cast<quint64>(thread), 0, 16));

            connect(this, &Model::step, uncoupled_train, &Train::slotStep);
            connect(uncoupled_train, &Train::stepDone, this, &Model::slotTrainStepDone);
            thread->start();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::loadInitData(init_data_t &init_data)
{
    Journal::instance()->info("==== Init data loading ====");

    CfgReader cfg;
    FileSystem &fs = FileSystem::getInstance();
    QString cfg_path = QString(fs.getConfigDir().c_str()) + fs.separator() + "init-data.xml";

    if (cfg.load(cfg_path))
    {
        QString secName = "InitData";

        if (!cfg.getString(secName, "RouteDirectory", init_data.route_dir_name))
        {
            init_data.route_dir_name = "experimental-polygon";
        }

        if (!cfg.getString(secName, "TrainConfig", init_data.train_config))
        {
            init_data.train_config = "vl60pk-1543";
        }

        if (!cfg.getString(secName, "TrajectoryName", init_data.trajectory_name))
        {
            init_data.trajectory_name = "route1_0001_1";
        }

        if (!cfg.getInt(secName, "Direction", init_data.direction))
        {
            init_data.direction = 1;
        }

        if (!cfg.getDouble(secName, "InitCoord", init_data.init_coord))
        {
            init_data.init_coord = 780.0;
        }

        if (!cfg.getDouble(secName, "InitVelocity", init_data.init_velocity))
        {
            init_data.init_velocity = 0.0;
        }

        if (!cfg.getDouble(secName, "CoeffToWheelRailFriction", init_data.coeff_to_wheel_rail_friction))
        {
            init_data.coeff_to_wheel_rail_friction = 1.0;
        }

        if (!cfg.getInt(secName, "IntegrationTimeInterval", init_data.integration_time_interval))
        {
            init_data.integration_time_interval = 15;
        }

        if (!cfg.getBool(secName, "DebugPrint", init_data.debug_print))
        {
            init_data.debug_print = false;
        }

        Journal::instance()->info("Loaded settings from: " + cfg_path);
    }
    else
    {
        Journal::instance()->error("File " + cfg_path + " not found");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::overrideByCommandLine(init_data_t &init_data,
                                  const simulator_command_line_t &command_line)
{
    Journal::instance()->info("==== Command line processing ====");

    if (command_line.start_datetime.is_present)
    {
        init_data.start_datetime = command_line.start_datetime.value;
    }

    if (command_line.route_dir.is_present)
    {
        init_data.route_dir_name = command_line.route_dir.value;
    }

    if (!command_line.train_config.is_present)
    {
        Journal::instance()->info("Command line is empty. Apply init_data.xml config");
        return;
    }

    init_data_t id;
    init_datas.clear();

    for (size_t i = 0; i < command_line.train_config.value.size(); ++i)
    {
        id.route_dir_name = init_data.route_dir_name;
        id.train_config = command_line.train_config.value[i];

        if (command_line.init_coord.is_present)
            id.init_coord = command_line.init_coord.value[i];

        if (command_line.direction.is_present)
            id.direction = command_line.direction.value[i];

        if (command_line.trajectory_name.is_present)
            id.trajectory_name = command_line.trajectory_name.value[i];

        init_datas.push_back(id);
    }

    Journal::instance()->info("Apply command line settinds");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::configSolver(solver_config_t &solver_config)
{
    Journal::instance()->info("==== Solver configuration ====");

    CfgReader cfg;
    FileSystem &fs = FileSystem::getInstance();
    QString cfg_path = QString(fs.getConfigDir().c_str()) + fs.separator() + "solver.xml";

    if (cfg.load(cfg_path))
    {
        QString secName = "Solver";

        if (!cfg.getString(secName, "Method", solver_config.method))
        {
            solver_config.method = "euler";
        }
        Journal::instance()->info("Integration method: " + solver_config.method);

        if (!cfg.getDouble(secName, "StartTime", solver_config.start_time))
        {
            solver_config.start_time = 0;
        }
        Journal::instance()->info("Start time: " + QString("%1").arg(solver_config.start_time));

        if (!cfg.getDouble(secName, "InitStep", solver_config.step))
        {
            solver_config.step = 3e-3;
        }
        Journal::instance()->info("Initial integration step: " + QString("%1").arg(solver_config.step));

        if (!cfg.getDouble(secName, "MaxStep", solver_config.max_step))
        {
            solver_config.max_step = 3e-3;
        }
        Journal::instance()->info("Maximal integration step: " + QString("%1").arg(solver_config.max_step));

        int tmp = 1;
        if (!cfg.getInt(secName, "SubStepNum", tmp))
        {
            solver_config.num_sub_step = 1;
        }
        else
        {
            solver_config.num_sub_step = static_cast<size_t>(tmp);
        }
        Journal::instance()->info("Number of substep: " + QString("%1").arg(solver_config.num_sub_step));

        if (!cfg.getDouble(secName, "LocalError", solver_config.local_error))
        {
            solver_config.local_error = 1e-5;
        }
        Journal::instance()->info("Local error of solution: " + QString("%1").arg(solver_config.local_error));
    }
    else
    {
        Journal::instance()->error("File " + cfg_path + " not found");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initControlPanel(QString cfg_path)
{
    CfgReader cfg;
    FileSystem &fs = FileSystem::getInstance();
    QString full_path = QString(fs.getConfigDir().c_str()) + fs.separator() + cfg_path + ".xml";

    if (cfg.load(full_path))
    {
        QString secName = "ControlPanel";

        bool is_allow = true;
        cfg.getBool(secName, "Allow", is_allow);
        if (!is_allow)
        {
            return;
        }

        int v_idx = 0;
        cfg.getInt(secName, "Vehicle", v_idx);
        if ((v_idx < 0) || v_idx >= vehicles.size())
        {
            return;
        }

        QString module_name = "";
        if (!cfg.getString(secName, "Plugin", module_name))
        {
            return;
        }

        control_panel = nullptr;
        QString module_path = QString(fs.getPluginsDir().c_str()) + fs.separator() + module_name;
        control_panel = loadInterfaceDevice(module_path);
        if (control_panel == nullptr)
        {
            return;
        }

        QString config_dir = "";
        cfg.getString(secName, "ConfigDir", config_dir);
        config_dir = QString(fs.toNativeSeparators(config_dir.toStdString()).c_str());
        config_dir = QString(fs.getConfigDir().c_str()) + fs.separator() + config_dir;
        if (!control_panel->init(config_dir))
        {
            return;
        }

        int request_interval = 0;
        if (!cfg.getInt(secName, "RequestInterval", request_interval))
            request_interval = 100;

        controlTimer.setInterval(request_interval);
        connect(&controlTimer, &QTimer::timeout, this, &Model::controlProcess);
        controlTimer.start();

        connect(this, &Model::sendSignalsToControlPanel,
                control_panel, &VirtualInterfaceDevice::receiveFeedback);

        connect(control_panel, &VirtualInterfaceDevice::sendControlSignals,
                this, &Model::receiveSignalsFromControlPanel);

        vehicle_controlled_by_panel = vehicles[v_idx];
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Train *Model::addTrain(const init_data_t &init_data)
{
    Journal::instance()->info("==== Train initialization ====");
    Train *train = new Train();
    train->setTopology(topology);
    Journal::instance()->info(QString("Created Train object at address: 0x%1")
                                  .arg(reinterpret_cast<quint64>(train), 0, 16));

    if (train->init(init_data, vehicles.size()))
    {
        Journal::instance()->info(QString("Train #%1 initialized successfully").arg(trains.size()));

        train->setTrainIndex(trains.size());
        for (auto vehicle : *(train->getVehicles()))
        {
            vehicle->setModelIndex(vehicles.size());
            vehicles.push_back(vehicle);
        }

        topology_pos_t tp;
        tp.traj_name = init_data.trajectory_name;
        tp.traj_coord = init_data.init_coord;
        tp.dir = init_data.direction;

        if (topology->addTrain(tp, train->getVehicles()))
        {
            Journal::instance()->info("Train added to topology successfully");
        }
        else
        {
            Journal::instance()->critical("CAN'T INITIALIZE TRAIN AT TOPOLOGY");
            delete train;
            return nullptr;
        }

        return train;
    }
    else
    {
        Journal::instance()->error("Can't initialize Train");
        return nullptr;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initTraffic(const init_data_t &init_data)
{
    traffic_machine = new TrafficMachine();

    FileSystem &fs = FileSystem::getInstance();
    std::string route_dir_path = fs.combinePath(fs.getRouteRootDir(), init_data.route_dir_name.toStdString());

    if (!traffic_machine->init(route_dir_path.c_str()))
    {
        Journal::instance()->error("Failed traffic initialization in route" +
                                   QString(route_dir_path.c_str()));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initTopology(const init_data_t &init_data)
{
    Journal::instance()->info("==== Route topology loading ====");

    if (topology->load(init_data.route_dir_name))
    {
        Journal::instance()->info("Loaded topology for route " + init_data.route_dir_name);
    }
    else
    {
        Journal::instance()->error("FAILED TOPOLOGY LOAD!!!");
        exit(0);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Model::initScenarioManager(const init_data_t &init_data,
                                const simulator_command_line_t &command_line)
{
    // Проверяем, есть ли вообще сценарий для исполнения
    if (!command_line.scenario.is_present)
    {
        return false;
    }

    // Инициализируем менеджер сценариев
    scnmgr->init(init_data);

    // Полный путь к скрипту сценария
    FileSystem &fs = FileSystem::getInstance();
    std::string script_path = fs.getRouteRootDir() + fs.separator()
                          + init_data.route_dir_name.toStdString() + fs.separator()
                          + "scenarios" + fs.separator() + command_line.scenario.value.toStdString() + ".lua";

    // Пытаемся выполнить скрипт
    if (!scnmgr->run(script_path))
    {
        return false;
    }

    // Увязываем управляющие сигналы с топологией
    connect(scnmgr, &ScenarioManager::setSwitchState, topology, &Topology::slotSetSwitchState);
    connect(scnmgr, &ScenarioManager::openSignal, topology, &Topology::slotOpenSignal);
    connect(scnmgr, &ScenarioManager::closeSignal, topology, &Topology::slotCloseSignal);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initTcpServer()
{
    Journal::instance()->info("==== TCP server initialization ====");

    FileSystem &fs = FileSystem::getInstance();
    std::string cfg_path = fs.getConfigDir() + fs.separator() + "tcp-server.xml";

    tcp_server->init(QString(cfg_path.c_str()));

    connect(tcp_server, &TcpServer::requestTopologyData, this, &Model::slotGetTopologyData);

    connect(tcp_server, &TcpServer::setSwitchState, topology, &Topology::slotSetSwitchState);
    connect(topology, &Topology::sendSwitchState, tcp_server, &TcpServer::slotSendSwitchState);

    connect(topology, &Topology::sendTrajBusyState, tcp_server, &TcpServer::slotSendTrajBusyState);

    connect(tcp_server, &TcpServer::requestSignalsData, this, &Model::slotGetSignalsData);

    for (auto signal : topology->getSignalsData()->line_signals)
    {
        connect(signal, &Signal::sendDataUpdate, tcp_server, &TcpServer::slotUpdateSignal);
    }

    for (auto signal : topology->getSignalsData()->enter_signals)
    {
        connect(signal, &Signal::sendDataUpdate, tcp_server, &TcpServer::slotUpdateSignal);
    }

    connect(tcp_server, &TcpServer::openSignal, topology, &Topology::slotOpenSignal);

    connect(tcp_server, &TcpServer::closeSignal, topology, &Topology::slotCloseSignal);

    for (auto signal : topology->getSignalsData()->exit_signals)
    {
        connect(signal, &Signal::sendDataUpdate, tcp_server, &TcpServer::slotUpdateSignal);
    }

    connect(tcp_server, &TcpServer::setVehicleControl, this, &Model::slotGetVehicleControlByKeyboard);

    connect(tcp_server, &TcpServer::resetVehicleControl, this, &Model::slotResetVehicleControlByKeyboard);

    Journal::instance()->info("TCP server is initialized successfully");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::prepareFeedBack()
{
    update_pos_data.vehicles.resize(vehicles.size());
    update_data.vehicles.resize(vehicles.size());
    update_data.trains.resize(trains.size());

    update_pos_data.sim_time = sim_time;

    int i = 0;
    for (auto train : trains)
    {
        update_data.trains[i].first_vehicle_id = train->getFirstVehicle()->getModelIndex();
        update_data.trains[i].last_vehicle_id = train->getLastVehicle()->getModelIndex();

        ++i;
    }

    i = 0;
    for (auto vehicle : vehicles)
    {
        profile_point_t *pp = vehicle->getProfilePoint();

        update_pos_data.vehicles[i].position_x = pp->position.x;
        update_pos_data.vehicles[i].position_y = pp->position.y;
        update_pos_data.vehicles[i].position_z = pp->position.z;
        update_pos_data.vehicles[i].orth_x = pp->orth.x;
        update_pos_data.vehicles[i].orth_y = pp->orth.y;
        update_pos_data.vehicles[i].orth_z = pp->orth.z;
        update_pos_data.vehicles[i].up_x = pp->up.x;
        update_pos_data.vehicles[i].up_y = pp->up.y;
        update_pos_data.vehicles[i].up_z = pp->up.z;

        update_data.vehicles[i].train_id = vehicle->getTrainIndex();
        int orient = vehicle->getOrientation();
        update_data.vehicles[i].orientation = orient;
        if (orient == -1)
        {
            update_data.vehicles[i].next_vehicle =
                (vehicle->getPrevVehicle() == nullptr) ?
                    -1 :
                    vehicle->getPrevVehicle()->getModelIndex();

            update_data.vehicles[i].prev_vehicle =
                (vehicle->getNextVehicle() == nullptr) ?
                    -1 :
                    vehicle->getNextVehicle()->getModelIndex();
        }
        else
        {
            update_data.vehicles[i].next_vehicle =
                (vehicle->getNextVehicle() == nullptr) ?
                    -1 :
                    vehicle->getNextVehicle()->getModelIndex();

            update_data.vehicles[i].prev_vehicle =
                (vehicle->getPrevVehicle() == nullptr) ?
                    -1 :
                    vehicle->getPrevVehicle()->getModelIndex();
        }

        update_data.vehicles[i].analogSignal = *(vehicle->getAnalogSignals());

        ++i;
    }

    // Раздаём соответствующие debug_msg по клиентам
    for (auto с_id = controlled_clients.keyBegin(); с_id != controlled_clients.keyEnd(); ++с_id)
    {
        update_players.clients_id.push_back(*с_id);

        int id = controlled_clients[*с_id].vehicle_control_by_keyboard.current_vehicle;
        update_players.current_vehicles.push_back(id);

        controlled_clients[*с_id].vehicle_controlled.current_vehicle = id;

        if (controlled_clients[*с_id].vehicle_control_by_keyboard.need_debug_msg)
        {
            controlled_clients[*с_id].vehicle_controlled.currentDebugMsg = vehicles[id]->getDebugMsg();
        }

        id = controlled_clients[*с_id].vehicle_control_by_keyboard.controlled_vehicle;
        update_players.controlled_vehicles.push_back(id);

        controlled_clients[*с_id].vehicle_controlled.controlled_vehicle = id;

        if (controlled_clients[*с_id].vehicle_control_by_keyboard.need_debug_msg)
        {
            controlled_clients[*с_id].vehicle_controlled.controlledDebugMsg = vehicles[id]->getDebugMsg();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::tcpFeedBack()
{
    tcp_server->updateVehiclesPos(update_pos_data.serialize(), sim_time.simulation_seconds);
    update_pos_data = simulator_update_pos_t();
    tcp_server->updateVehiclesState(update_data.serialize(), sim_time.simulation_seconds);
    update_data = simulator_update_t();
    tcp_server->updatePlayers(update_players.serialize(), sim_time.simulation_seconds);
    update_players = simulator_update_players_t();

    for (auto с_id = controlled_clients.keyBegin(); с_id != controlled_clients.keyEnd(); ++с_id)
    {
        tcp_server->updateVehicleControlled(controlled_clients[*с_id].vehicle_controlled.serialize(), (*с_id), sim_time.simulation_seconds);
        controlled_clients[*с_id].vehicle_controlled = simulator_vehicle_controlled_update_t();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::controlStep()
{
    for (const auto& c : controlled_clients)
    {
        int id = c.prev_vehicle_controlled;
        if ((id >= 0) && (id < vehicles.size()))
        {
            int cab_id = c.prev_cab_controlled;
            if (cab_id >= 0)
            {
                vehicles[id]->resetKeyboardControl(cab_id);
            }

            vehicles[id]->setNeedDebugMsg(false);
        }

        id = c.prev_vehicle_current;
        if ((id >= 0) && (id < vehicles.size()))
        {
            vehicles[id]->setNeedDebugMsg(false);
        }
    }

    for (const auto& c : controlled_clients)
    {
        std::uint16_t id = c.vehicle_control_by_keyboard.controlled_vehicle;
        if (id < vehicles.size())
        {
            std::uint16_t cab_id = c.vehicle_control_by_keyboard.controlled_cabine_idx;
            vehicles[id]->setKeyboardControl(cab_id, c.vehicle_control_by_keyboard.pressed_keys);
            vehicles[id]->setNeedDebugMsg(c.vehicle_control_by_keyboard.need_debug_msg);
        }
        id = c.vehicle_control_by_keyboard.current_vehicle;
        if (id < vehicles.size())
        {
            vehicles[id]->setNeedDebugMsg(c.vehicle_control_by_keyboard.need_debug_msg);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::process()
{
    // Проверяем, если в счётчике ещё нет отрицательного значения,
    // то предыдущий шаг симуляции не завершён, пропускаем новый шаг
    if (count_trains_done_its_step >= 0)
    {
        Journal::instance()->critical("WARNING: skip step because previous not done yet");
        return;
    }
    // Обнуляем счётчик
    count_trains_done_its_step = 0;

    realtime_at_step_begin = QTime::currentTime().msecsSinceStartOfDay();

    double integration_time = static_cast<double>(integration_time_interval) / 1000.0;

    topology->step(sim_time.simulation_seconds, integration_time);

    findNearestVehicles();

    findFarthestVehicles();

    prepareFeedBack();

    controlStep();

    emit step(sim_time, integration_time);

    // Update server feedback
    tcpFeedBack();

    sim_time.addTime(integration_time);
    //Journal::instance()->info(sim_time.getString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotTrainStepDone(int idx)
{
    // Считаем количество поездов, завершивших шаг симуляции
    ++count_trains_done_its_step;

    // Проверяем, что все поезда закончили шаг
    if (count_trains_done_its_step >= trains.size())
    {
        // Отрицательное значение счётчика как признак завершения шага
        count_trains_done_its_step = -1;

        // Расчитываем задержку симуляции от реалтайма
        int realtime_at_step_end = QTime::currentTime().msecsSinceStartOfDay();
        realtime_delay = realtime_at_step_end - realtime_at_step_begin - integration_time_interval;
        if (realtime_delay > 0)
        {
            QString msg = QString("t = %1 | simulation of %2ms take %3ms | slowest train #%4 | WARNING: realtime delay!")
                              .arg(sim_time.simulation_seconds, 10, 'f', 3)
                              .arg(integration_time_interval)
                              .arg(realtime_delay + integration_time_interval)
                              .arg(idx);
            fputs(qPrintable(msg + "\n"), stdout);
            Journal::instance()->critical(msg);
        }/*
        else
        {
            QString msg = QString("t = %1 | simulation of %2ms take %3ms | slowest train #%4 ")
                              .arg(sim_time.simulation_seconds, 10, 'f', 3)
                              .arg(integration_time_interval)
                              .arg(realtime_delay + integration_time_interval)
                              .arg(idx);
            fputs(qPrintable(msg + "\n"), stdout);
            Journal::instance()->critical(msg);
        }*/
    }/*
    else
    {
        Journal::instance()->critical(QString("t = %1 | wait to step: %2/%3 trains done | last #%4")
                                          .arg(sim_time.simulation_seconds, 10, 'f', 3)
                                          .arg(count_trains_done_its_step)
                                          .arg(trains.size())
                                          .arg(idx));
    }*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotGetTopologyData(QByteArray &topology_data)
{
    topology_data = topology->serialize();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotGetSignalsData(QByteArray &signals_data)
{
    signals_data = topology->getSignalsData()->serialize();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotGetVehicleControlByKeyboard(QByteArray &control_data, int client_id)
{
    controlled_client_t c = controlled_client_t();
    c.vehicle_control_by_keyboard.deserialize(control_data);
    if (controlled_clients.contains(client_id))
    {
        c.prev_vehicle_controlled = controlled_clients[client_id].vehicle_control_by_keyboard.controlled_vehicle;
        c.prev_cab_controlled = controlled_clients[client_id].vehicle_control_by_keyboard.controlled_cabine_idx;
    }
    controlled_clients.insert(client_id, c);
/*
    QString msg = "Get keyboard: controlled ";
    msg += QString::number(c.vehicle_control_by_keyboard.controlled_vehicle);
    msg += " | current ";
    msg += QString::number(c.vehicle_control_by_keyboard.current_vehicle);
    msg += " | cabine ";
    msg += QString::number(c.vehicle_control_by_keyboard.controlled_cabine_idx);
    msg += " | keys: ";
    msg += QString::number(c.vehicle_control_by_keyboard.pressed_keys.size());
    for (auto key_id : c.vehicle_control_by_keyboard.pressed_keys)
    {
        msg += " | ";
        msg += QString::number(key_id);
    }
    Journal::instance()->info(msg);
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotResetVehicleControlByKeyboard(int client_id)
{
    if (controlled_clients.contains(client_id))
    {
        int id = controlled_clients[client_id].prev_vehicle_controlled;
        if ((id >= 0) && (id < vehicles.size()))
        {
            int cab_id = controlled_clients[client_id].prev_cab_controlled;
            if (cab_id >= 0)
                vehicles[id]->resetKeyboardControl(cab_id);

            if (controlled_clients[client_id].vehicle_control_by_keyboard.need_debug_msg)
                vehicles[id]->setNeedDebugMsg(false);
        }

        id = controlled_clients[client_id].prev_vehicle_current;
        if ((id >= 0) && (id < vehicles.size()))
        {
            if (controlled_clients[client_id].vehicle_control_by_keyboard.need_debug_msg)
                vehicles[id]->setNeedDebugMsg(false);
        }

        controlled_clients.remove(client_id);
    }
}
