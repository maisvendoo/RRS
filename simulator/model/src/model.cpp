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
#include "rail-signal.h"

#include    <CfgReader.h>
#include    <Journal.h>
#include    <JournalFile.h>
#include    <vehicle-controller.h>
#include    <core/load_module.h>

#include    <QFile>
#include    <QTextStream>

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
    if (!initScenarioManager(init_data, command_line))
    {
        Journal::instance()->critical("Failed scenario manager initialization");
        return false;
    }

    if (!scnmgr->init_datas.empty())
    {
        init_datas = scnmgr->init_datas;
    }
    else
    {
        Journal::instance()->critical("Train's list is empty!!!");
        return false;
    }

    init_data.start_datetime = scnmgr->getStartDateTime();

    // Create all trains
    for (size_t i = 0; i < init_datas.size(); ++i)
    {
        Train *train = addTrain(init_datas[i]);

        if (train)
        {
            // Передаем начальные индексы поездов менеджеру сценариев
            size_t train_idx = train->getTrainIndex();
            scnmgr->setTrainIndex(train_idx);

            // Даем начальное имя поезду
            train->setName(scnmgr->getTrainName(train_idx));

            buildAutostartQueue(train);

            QThread *thread = new QThread();
            train_threads.push_back(thread);
            train->moveToThread(thread);
            Journal::instance()->info(QString("Created new thread for train at address: 0x%1")
                                          .arg(reinterpret_cast<quint64>(thread), 0, 16));

            connect(this, &Model::step, train, &Train::slotStep);
            connect(train, &Train::stepDone, this, &Model::slotTrainStepDone);

            slotUpdateTrainTimetable(train_idx);

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

    tcp_server->setStationsData(topology->serialize_stations());
    Journal::instance()->info("Ready stations data for server");

    simulator_vehicles_info_t vehicles_info;
    vehicles_info.vehicles.resize(vehicles.size());
    size_t i = 0;
    for (auto it = vehicles.begin(); it != vehicles.end(); ++it)
    {
        vehicles_info.vehicles[i].vehicle_length = (*it)->getLength();
        vehicles_info.vehicles[i].vehicle_config_dir = (*it)->getConfigDir();
        vehicles_info.vehicles[i].vehicle_config_file = (*it)->getConfigName();

        connect(*it, &Vehicle::sigGetTrainParams, this, &Model::slotGetTrainParams);

        ++i;
    }
    tcp_server->setVehiclesInfo(vehicles_info.serialize());
    Journal::instance()->info("Ready vehicles info for server");

    update_pos_data.vehicles.resize(vehicles.size());
    update_vehicles.vehicles.resize(vehicles.size());

    prepareFeedBack(true);
    tcpFeedBack(true);

    Journal::instance()->info("Ready trains and vehicles state for server");

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

        double interval = static_cast<double>(integration_time_interval);
        if (init_datas[0].simulation_speed > Physics::ZERO)
        {
            interval = interval / init_datas[0].simulation_speed;
        }
        simTimer.setInterval(static_cast<quint64>(std::ceil(interval)));
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
std::vector<std::size_t> Model::getControlledVehiclesInTrain(size_t train_idx)
{
    std::vector<std::size_t> veh_indexes;

    // Просматриваем управление от всех клиентов
    for (const auto& cc : controlled_clients)
    {
        // Управляемая данным клиентом ПЕ
        std::size_t veh_idx = cc.vehicle_control_by_keyboard.controlled_vehicle;
        if (veh_idx < vehicles.size())
        {
            Vehicle* veh = vehicles[veh_idx];

            // Если ПЕ находится в данном поезде, сохраняем её
            if (veh->getTrainIndex() == train_idx)
            {
                veh_indexes.push_back(veh_idx);
            }
        }
    }
    return veh_indexes;
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
void Model::slotSetSimSpeed(int speed_factor)
{
    if (init_datas.empty())
    {
        return;
    }

    if (speed_factor < 0)
    {
        return;
    }

    this->speed_factor = speed_factor;

    if (this->speed_factor == 0)
    {
        simTimer.setInterval(integration_time_interval);
    }
    else
    {
        quint64 interval = qRound(static_cast<double>(integration_time_interval) / this->speed_factor);

        if (interval < 1)
        {
            interval = 1;
        }

        simTimer.setInterval(interval);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::buildAutostartQueue(Train *train)
{
    if (train == nullptr)
    {
        return;
    }

    if (scnmgr->isTrainAutostarted(train->getTrainIndex()))
    {
        for (auto vehicle : *(train->getVehicles()))
        {
            if (!vehicle->getAutopilot().empty())
            {
                vehicles_for_autostart.push(vehicle);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotUpdateTrainTimetable(int train_idx)
{
    if (train_idx >= trains.size())
    {
        return;
    }

    auto train = trains[train_idx];

    autopilot_timetable_t timetable = scnmgr->loadTrainTimetable(train_idx);

    for (auto vehicle : *(train->getVehicles()))
    {
        if (!vehicle->getAutopilot().empty())
        {
            for (auto ap : vehicle->getAutopilot())
            {
                ap->setTimetable(timetable);

                auto& vc = topology->getVehicleController(vehicle->getModelIndex());

                disconnect(ap, &Autopilot::sigGetVehicleTrajPosition, &vc, &VehicleController::slotGetVehicleTrajPosition);
                disconnect(ap, &Autopilot::sigIsRouteExists, topology, &Topology::slotIsRouteExists);
                disconnect(ap, &Autopilot::sigGetRouteLength, topology, &Topology::slotGetRouteLength);
                disconnect(this, &Model::sigInitTimetable, ap, &Autopilot::slotInitTimeTable);
                disconnect(topology, &Topology::sigIncTargetStation, ap, &Autopilot::slotIncTargetStation);
                disconnect(topology, &Topology::sigCalcMiddleVelocity, ap, &Autopilot::slotCalcMiddleVelocity);
                disconnect(scnmgr, &ScenarioManager::sigSetTimeForAutopilot, ap, &Autopilot::slotSetTimeForAutopilot);
                disconnect(ap, &Autopilot::sigBuildTrainRoute, scnmgr, &ScenarioManager::slotBuildTrainRoute);

                disconnect(ap, &Autopilot::sigGetTrajStateRequest, topology, &Topology::slotGetTrajStateRequest);
                disconnect(topology, &Topology::sigGetTrajState, ap, &Autopilot::slotGetTrajState);

                if (!timetable.stations.empty())
                {
                    connect(ap, &Autopilot::sigGetVehicleTrajPosition, &vc, &VehicleController::slotGetVehicleTrajPosition);
                    connect(ap, &Autopilot::sigIsRouteExists, topology, &Topology::slotIsRouteExists);
                    connect(ap, &Autopilot::sigGetRouteLength, topology, &Topology::slotGetRouteLength);
                    connect(this, &Model::sigInitTimetable, ap, &Autopilot::slotInitTimeTable);
                    connect(topology, &Topology::sigIncTargetStation, ap, &Autopilot::slotIncTargetStation);
                    connect(topology, &Topology::sigCalcMiddleVelocity, ap, &Autopilot::slotCalcMiddleVelocity);
                    connect(ap, &Autopilot::sigBuildTrainRoute, scnmgr, &ScenarioManager::slotBuildTrainRoute);
                    connect(scnmgr, &ScenarioManager::sigSetTimeForAutopilot, ap, &Autopilot::slotSetTimeForAutopilot);

                    connect(ap, &Autopilot::sigGetTrajStateRequest, topology, &Topology::slotGetTrajStateRequest);
                    connect(topology, &Topology::sigGetTrajState, ap, &Autopilot::slotGetTrajState);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::processAutostartQueue()
{
    if (vehicles_for_autostart.empty())
    {
        return;
    }

    Vehicle *vehicle = std::move(vehicles_for_autostart.front());
    vehicles_for_autostart.pop();

    if (vehicle == nullptr)
    {
        return;
    }

    vehicle->OnAutopilot();
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

        // От каждого поезда ищем вперёд и назад по топологии
        std::tuple<int, int, bool> vehicles_idx_and_directions[] =
        {
            {train->getFirstVehicle()->getModelIndex(), train->getFirstVehicle()->getDirection(), true},
            {train->getLastVehicle()->getModelIndex(), -(train->getLastVehicle()->getDirection()), false}
        };
        for (auto [idx, veh_dir, is_train_head] : vehicles_idx_and_directions)
        {
            // Ищем другую ПЕ в пределах 10 метров, и дистанцию до неё в данный момент
            double current_distance = 0.0;
            dir_t search_dir = static_cast<dir_t>(veh_dir);
            int nearest_idx = topology->getVehicleController(idx).getNearestVehicle(
                current_distance, DISTANCE_TO_COUPLE_TRAINS, search_dir);

            // Если ничего не нашли - дальше делать нечего
            if (nearest_idx == -1)
            {
                train->setDistanceToEndOfTrajectory(is_train_head, current_distance);
                continue;
            }

            train->setDistanceToEndOfTrajectory(is_train_head, DISTANCE_TO_COUPLE_TRAINS);

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
                                              .arg(is_train_head ? "head" : "tail"));
                // Новый сцеп
                trains[fd.train_idx]->couple(current_distance, fd.from_head, is_train_head, train);
                // Сбрасываем имя поезда
                trains[fd.train_idx]->setName("");

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
                fd.from_head = is_train_head;
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

        // Удаляем поезда из контекста сценария
        scnmgr->deleteTrainByIndex(train_idx);
    }

    // Назначаем новые порядковые индексы поездам после уменьшения массива
    for (size_t train_idx = trains_idx_to_delete.back(); train_idx < trains.size(); ++train_idx)
    {
        Journal::instance()->info(QString("Train #%1 now #%2")
                                      .arg(trains[train_idx]->getTrainIndex(), 3)
                                      .arg(train_idx, 3));
        trains[train_idx]->setTrainIndex(train_idx);

        // Те же индексы сообщаем менеджеру сценариев
        scnmgr->setTrainIndex(train_idx);
    }

    is_trains_changed = true;
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

            // Добавляем наш новый поезд в контекст менеджера сценариев
            scenario_train_data_t scn_train;
            scn_train.setIndex(uncoupled_train->getTrainIndex());
            scnmgr->addNewTrain(scn_train);

            QThread *thread = new QThread();
            train_threads.push_back(thread);
            uncoupled_train->moveToThread(thread);
            Journal::instance()->info(QString("Created new thread for train at address: 0x%1")
                                          .arg(reinterpret_cast<quint64>(thread), 0, 16));

            connect(this, &Model::step, uncoupled_train, &Train::slotStep);
            connect(uncoupled_train, &Train::stepDone, this, &Model::slotTrainStepDone);
            thread->start();

            is_trains_changed = true;
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

        if (!cfg.getBool(secName, "LuaDebug", init_data.lua_debug))
        {
            init_data.lua_debug = false;
        }

        if (!cfg.getDouble(secName, "SimulationSpeed", init_data.simulation_speed))
        {
            init_data.simulation_speed = 1.0;
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
        control_panel = LOAD_MODULE(VirtualInterfaceDevice, module_path);
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

        const size_t initial_veh_count = vehicles.size();
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

            train->setTrainIndex(trains.size());
            trains.push_back(train);

            return train;
        }

        Journal::instance()->critical("CAN'T INITIALIZE TRAIN AT TOPOLOGY");
        for (auto it = vehicles.begin() + initial_veh_count; it != vehicles.end(); ++it)
        {
            delete *it;
        }
        vehicles.erase(vehicles.begin() + initial_veh_count, vehicles.end());
        delete train;
        return nullptr;
    }

    Journal::instance()->error("Can't initialize Train");
    return nullptr;
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
    // Инициализируем менеджер сценариев
    scnmgr->init(init_data);

    // Увязываем управляющие сигналы с топологией
    connect(scnmgr, &ScenarioManager::sigGetSwitchState, topology, &Topology::slotGetSwitchState);
    connect(scnmgr, &ScenarioManager::sigSwitchCommand, topology, &Topology::slotSwitchCommand);
    connect(scnmgr, &ScenarioManager::sigSignalCommand, topology, &Topology::slotSignalCommand);
    connect(scnmgr, &ScenarioManager::sigSetSwitchsAlongRoute, topology, &Topology::slotBuildRouteCommand);
    connect(scnmgr, &ScenarioManager::sigBuildTrainRoute, topology, &Topology::slotTrainRouteCommand);
    connect(scnmgr, &ScenarioManager::sigBuildShuntingRoute, topology, &Topology::slotShuntingRouteCommand);
    connect(topology, &Topology::sigSetOpenSignalsQueue, scnmgr, &ScenarioManager::slotSetOpenSignalsQueue);
    connect(tcp_server, &TcpServer::sigRenameTrain, scnmgr, &ScenarioManager::slotRenameTrain);
    connect(scnmgr, &ScenarioManager::sigRenameTrainInModel, this, &Model::slotRenameTrainInModel);
    connect(topology, &Topology::sigChangeTrajStateByTrain, scnmgr, &ScenarioManager::slotChangeTrajStateByTrain);
    connect(scnmgr, &ScenarioManager::sigGetTrajState, topology, &Topology::slotGetTrajState);
    connect(scnmgr, &ScenarioManager::sigGetNextTrajName, topology, &Topology::slotGetNextTrajName);
    connect(scnmgr, &ScenarioManager::sigUpdateTrainTimetable, this, &Model::slotUpdateTrainTimetable);

    // Проверяем, есть ли вообще сценарий для исполнения
    if (!command_line.scenario.is_present)
    {
        return false;
    }

    // Пытаемся выполнить скрипт
    if (!scnmgr->run(init_data.route_dir_name.toStdString(),
                     command_line.scenario.value.toStdString()))
    {
        return false;
    }

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

    connect(tcp_server, &TcpServer::requestTopologyModules, this, &Model::slotGetTopologyModules);

    connect(topology, &Topology::sendTrajBusyState, tcp_server, &TcpServer::slotSendTrajBusyState);

    connect(topology, &Topology::sendModuleUpdate, tcp_server, &TcpServer::slotSendTopologyModuleState);

    connect(topology, &Topology::sendSwitchState, tcp_server, &TcpServer::slotSendSwitchState);

    connect(tcp_server, &TcpServer::requestSignalsData, this, &Model::slotGetSignalsData);

    for (auto signal : topology->getSignalsData()->line_signals)
    {
        connect(signal, &Signal::sendDataUpdate, tcp_server, &TcpServer::slotUpdateSignal);
    }

    for (auto signal : topology->getSignalsData()->enter_signals)
    {
        connect(signal, &Signal::sendDataUpdate, tcp_server, &TcpServer::slotUpdateSignal);
    }

    for (auto signal : topology->getSignalsData()->route_signals)
    {
        connect(signal, &Signal::sendDataUpdate, tcp_server, &TcpServer::slotUpdateSignal);
    }

    for (auto signal : topology->getSignalsData()->exit_signals)
    {
        connect(signal, &Signal::sendDataUpdate, tcp_server, &TcpServer::slotUpdateSignal);
    }

    for (auto signal : topology->getSignalsData()->shunt_signals)
    {
        connect(signal, &Signal::sendDataUpdate, tcp_server, &TcpServer::slotUpdateSignal);
    }

    connect(tcp_server, &TcpServer::sigSwitchCommand, topology, &Topology::slotSwitchCommand);

    connect(tcp_server, &TcpServer::sigSignalCommand, topology, &Topology::slotSignalCommand);

    connect(tcp_server, &TcpServer::sigBuildRouteCommand, topology, &Topology::slotBuildRouteCommand);

    connect(tcp_server, &TcpServer::sigTrainRouteCommand, topology, &Topology::slotTrainRouteCommand);

    connect(tcp_server, &TcpServer::sigShuntingRouteCommand, topology, &Topology::slotShuntingRouteCommand);

    connect(tcp_server, &TcpServer::sigVehicleControl, this, &Model::slotGetVehicleControlByKeyboard);

    connect(tcp_server, &TcpServer::sigResetVehicleControl, this, &Model::slotResetVehicleControlByKeyboard);

    connect(tcp_server, &TcpServer::sigRenameTrain, this, &Model::slotRenameTrainInModel);

    connect(tcp_server, &TcpServer::sigReverseTrain, this, &Model::slotReverseTrain);

    connect(tcp_server, &TcpServer::sigSetSimSpeed, this, &Model::slotSetSimSpeed);

    Journal::instance()->info("TCP server is initialized successfully");    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::prepareFeedBack(bool need_trains_feedback)
{
    int i = 0;
    if (need_trains_feedback)
    {
        update_trains.trains.resize(trains.size());
        for (auto train : trains)
        {
            update_trains.trains[i].first_vehicle_id = train->getFirstVehicle()->getModelIndex();
            update_trains.trains[i].last_vehicle_id = train->getLastVehicle()->getModelIndex();
            update_trains.trains[i].train_name = QString(train->getName().c_str());

            ++i;
        }
    }

    update_pos_data.speed_factor = speed_factor;
    update_pos_data.sim_time = sim_time;
    //update_pos_data.vehicles.resize(vehicles.size());
    //update_vehicles.vehicles.resize(vehicles.size());
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

        update_vehicles.vehicles[i].train_id = vehicle->getTrainIndex();
        int orient = vehicle->getDirection();
        update_vehicles.vehicles[i].orientation = orient;
        if (orient == -1)
        {
            update_vehicles.vehicles[i].next_vehicle =
                (vehicle->getPrevVehicle() == nullptr) ?
                    -1 :
                    vehicle->getPrevVehicle()->getModelIndex();

            update_vehicles.vehicles[i].prev_vehicle =
                (vehicle->getNextVehicle() == nullptr) ?
                    -1 :
                    vehicle->getNextVehicle()->getModelIndex();
        }
        else
        {
            update_vehicles.vehicles[i].next_vehicle =
                (vehicle->getNextVehicle() == nullptr) ?
                    -1 :
                    vehicle->getNextVehicle()->getModelIndex();

            update_vehicles.vehicles[i].prev_vehicle =
                (vehicle->getPrevVehicle() == nullptr) ?
                    -1 :
                    vehicle->getPrevVehicle()->getModelIndex();
        }

        update_vehicles.vehicles[i].analogSignal = *(vehicle->getAnalogSignals());

        if (!vehicle->getAutopilot().empty())
        {
            update_vehicles.vehicles[i].timetableData = vehicle->getAutopilot().at(0)->getTimetableData();
        }

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
void Model::prepareProfilesFeedback()
{
    // Дальности профиля - максимум запросов всех подписчиков
    double backward_m = 4000.0;
    double forward_m = 4000.0;
    tcp_server->getTrainProfileExtents(backward_m, forward_m);

    update_profiles.clear();
    update_profiles.reserve(trains.size());

    for (size_t i = 0; i < trains.size(); ++i)
    {
        Train* train = trains[i];

        std::vector<Vehicle*>* vlist = train->getVehicles();
        if ((vlist == nullptr) || vlist->empty())
            continue;

        // Средняя ПЕ поезда - точка отсчёта профиля
        Vehicle* mid_vehicle = (*vlist)[vlist->size() / 2];

        VehicleController& vc= topology->getVehicleController(mid_vehicle->getModelIndex());

        QString traj_name;
        double coord = 0.0;
        vc.slotGetVehicleTrajPosition(&traj_name, &coord);
        dir_t orient = static_cast<dir_t>(vc.getOrientation() * mid_vehicle->getDirection());

        Trajectory* traj = topology->getTrajectoriesList()->value(traj_name);
        if (traj == nullptr)
            continue;

        profile_segments_t profile;
        if (!topology->getProfile(traj, coord, orient, backward_m, forward_m, profile))
            continue;

        if (profile.points.empty())
            continue;

        simulator_train_profile_update_t upd;
        upd.train_id = static_cast<int>(i);
        upd.middle_vehicle_id = static_cast<int>(mid_vehicle->getModelIndex());
        upd.direction = static_cast<int>(orient);
        upd.speed = static_cast<float>(mid_vehicle->getVelocity());
        upd.backward = static_cast<float>(profile.backward);
        upd.forward = static_cast<float>(profile.forward);
        upd.backward_requested = static_cast<float>(backward_m);
        upd.forward_requested = static_cast<float>(forward_m);

        upd.profile.reserve(profile.points.size());
        for (const profile_segment_t& p : profile.points)
        {
            simulator_train_profile_point_t point;
            point.distance = static_cast<float>(p.distance);
            point.elevation = static_cast<float>(p.elevation);
            point.railway_coord = static_cast<float>(p.railway_coord);
            point.inclination = static_cast<float>(p.inclination);
            upd.profile.push_back(point);
        }

        // Единицы подвижного состава на профиле (включая вагоны других поездов)
        upd.vehicles.reserve(profile.vehicles.size());
        for (const profile_vehicle_t& pv : profile.vehicles)
        {
            simulator_train_profile_vehicle_t vehicle;
            vehicle.vehicle_id = static_cast<int>(pv.vehicle_id);
            vehicle.begin_distance = static_cast<float>(pv.begin_distance);
            vehicle.end_distance = static_cast<float>(pv.end_distance);
            upd.vehicles.push_back(vehicle);
        }

        // Светофоры на профиле (попутные по ходу движения поезда)
        upd.signal_list.reserve(profile.signal_list.size());
        for (const profile_signal_t& ps : profile.signal_list)
        {
            simulator_train_profile_signal_t signal;
            signal.distance = static_cast<float>(ps.distance);
            signal.connector_name = ps.connector_name;
            signal.signal_dir = ps.signal_dir;
            upd.signal_list.push_back(signal);
        }

        // Станции на профиле
        upd.stations.reserve(profile.stations.size());
        for (const profile_station_t& pst : profile.stations)
        {
            simulator_train_profile_station_t station;
            station.distance = static_cast<float>(pst.distance);
            station.name = pst.name;
            upd.stations.push_back(station);
        }

        // Ограничения скорости на профиле
        upd.speed_limits.reserve(profile.speed_limits.size());
        for (const profile_speed_limit_t& psl : profile.speed_limits)
        {
            simulator_train_profile_speed_limit_t sl;
            sl.distance = static_cast<float>(psl.distance);
            sl.end_distance = static_cast<float>(psl.end_distance);
            sl.speed_kmh = static_cast<float>(psl.speed_kmh);
            upd.speed_limits.push_back(sl);
        }

        update_profiles.push_back(upd);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::tcpFeedBack(bool need_trains_feedback)
{
    if (need_trains_feedback)
    {
        tcp_server->updateTrainsInfo(update_trains.serialize());
        update_trains = simulator_trains_update_t();
    }

    double realtime_seconds = std::chrono::duration<double, std::chrono::seconds::period>(process_timepoint - start_timepoint).count();
    tcp_server->updateVehiclesPos(update_pos_data.serialize(), realtime_seconds);
    //update_pos_data = simulator_update_pos_t();

    tcp_server->updateVehiclesState(update_vehicles.serialize(), realtime_seconds);
    //update_vehicles = simulator_vehicles_update_t();

    tcp_server->updatePlayers(update_players.serialize(), realtime_seconds);
    update_players = simulator_update_players_t();

    // Профили путей поездов: пересчёт и рассылка не чаще заданного интервала
    if (tcp_server->hasTrainProfileSubscribers() &&
        (realtime_seconds - profiles_update_prev_time) > profiles_update_interval)
    {
        profiles_update_prev_time = realtime_seconds;
        prepareProfilesFeedback();
        for (const auto& profile : update_profiles)
        {
            tcp_server->updateTrainProfile(profile.serialize(), realtime_seconds);
        }
    }

    for (auto с_id = controlled_clients.keyBegin(); с_id != controlled_clients.keyEnd(); ++с_id)
    {
        tcp_server->updateVehicleControlled(controlled_clients[*с_id].vehicle_controlled.serialize(), (*с_id), realtime_seconds);
        controlled_clients[*с_id].vehicle_controlled = simulator_vehicle_controlled_update_t();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::controlStep()
{
    // Сбрасываем предыдущее управление и требования выводить дебаг-строку
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

    // Задаём новое управление и требования выводить дебаг-строку
    for (const auto& c : controlled_clients)
    {
        std::uint16_t id = c.vehicle_control_by_keyboard.controlled_vehicle;
        if (id < vehicles.size())
        {
            std::uint16_t cab_id = c.vehicle_control_by_keyboard.controlled_cabine_idx;
            vehicles[id]->setKeyboardControl(cab_id, c.vehicle_control_by_keyboard.pressed_keys);

            if (c.vehicle_control_by_keyboard.need_debug_msg)
            {
                vehicles[id]->setNeedDebugMsg(true);

                id = c.vehicle_control_by_keyboard.current_vehicle;
                if (id < vehicles.size())
                {
                    vehicles[id]->setNeedDebugMsg(true);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::process()
{
    process_timepoint = std::chrono::steady_clock::now();

    if (speed_factor == 0)
    {
        prepareFeedBack(false);
        tcpFeedBack(false);
        return;
    }

    // Проверяем, если в счётчике ещё нет отрицательного значения,
    // то предыдущий шаг симуляции не завершён, пропускаем новый шаг
    if (count_trains_done_its_step >= 0)
    {
        Journal::instance()->critical("WARNING: skip step because previous not done yet");
        return;
    }
    // Обнуляем счётчик
    count_trains_done_its_step = 0;

    double integration_time = static_cast<double>(integration_time_interval) / 1000.0;

    topology->step(sim_time.simulation_seconds, integration_time);

    emit sigInitTimetable();

    scnmgr->step(sim_time, integration_time);

    // Обрабатываем очередь на автозапуск
    processAutostartQueue();

    findNearestVehicles();

    findFarthestVehicles();

    bool need_trains_feedback = is_trains_changed;
    is_trains_changed = false;

    prepareFeedBack(need_trains_feedback);

    controlStep();

    emit step(sim_time, integration_time);

    // Update server feedback
    tcpFeedBack(need_trains_feedback);

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
        std::chrono::steady_clock::time_point end_timepoint = std::chrono::steady_clock::now();
        realtime_delay = std::chrono::duration<double, std::chrono::seconds::period>(end_timepoint - process_timepoint).count();
        if (realtime_delay * 1000.0 > integration_time_interval)
        {
            QString msg = QString("t = %1 | simulation of %2ms take %3ms | slowest train #%4 | WARNING: realtime delay!")
                              .arg(sim_time.simulation_seconds, 10, 'f', 3)
                              .arg(integration_time_interval)
                              .arg(realtime_delay * 1000.0, 10, 'f', 1)
                              .arg(idx);
            fputs(qPrintable(msg + "\n"), stdout);
            Journal::instance()->critical(msg);
        }/*
        else
        {
            QString msg = QString("t = %1 | simulation of %2ms take %3ms | slowest train #%4 ")
                              .arg(sim_time.simulation_seconds, 10, 'f', 3)
                              .arg(integration_time_interval)
                              .arg(realtime_delay * 1000.0, 10, 'f', 1)
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
void Model::slotGetTopologyModules(QByteArray &topology_modules)
{
    topology_modules = topology->serialize_modules();
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotRenameTrainInModel(int train_idx, QString new_name)
{
    size_t t_idx = static_cast<size_t>(train_idx);

    if (t_idx >= trains.size())
    {
        Journal::instance()->error(QString("Rename train: Train index out of range (%1/%2)").arg(t_idx).arg(trains.size()));
        return;
    }

    trains[t_idx]->setName(new_name.toStdString());

    Journal::instance()->info(QString("Rename train: Train #%1 has new name %2").arg(t_idx).arg(new_name));

    is_trains_changed = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotReverseTrain(int train_idx)
{
    size_t t_idx = static_cast<size_t>(train_idx);

    if (t_idx >= trains.size())
    {
        Journal::instance()->error(QString("Reverse train: Train index out of range (%1/%2)").arg(t_idx).arg(trains.size()));
        return;
    }

    trains[t_idx]->reverse();

    Journal::instance()->info(QString("Reverse train #%1").arg(t_idx));

    is_trains_changed = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::slotGetTrainParams(int train_idx, double &train_len, double &train_mass)
{
    if (train_idx >= trains.size())
    {
        return;
    }

    auto train  = trains[train_idx];

    if (train == nullptr)
    {
        return;
    }

    train_len = train->getLength();
    train_mass = train->getMass();
}


