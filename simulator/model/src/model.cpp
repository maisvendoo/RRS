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

#include    <algorithm>
#include    <cmath>
#include    <map>

#include    <CfgReader.h>
#include    <Journal.h>
#include    <JournalFile.h>
#include    <vehicle-controller.h>
#include    <core/load_module.h>

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

    // Init collision world
    initCollisionWorld(init_data);

    // Init route electrification (catenary, substations)
    initElectrification(init_data);

    // Init weather (ТЗ "Видимость и погода")
    {
        FileSystem &fs = FileSystem::getInstance();
        const std::string route_dir = fs.getRouteRootDir() + "/" +
            init_data.route_dir_name.toStdString();

        weather_system.load(QString::fromStdString(route_dir));

        Journal::instance()->info("==== Weather loaded ====");
    }

    // Точки погрузки/разгрузки (ТЗ "Погрузка")
    initLoadingPoints(init_data);

    // Зоны тоннелей и заправочных колонок (ТЗ "43-47", "Снабжение")
    initTunnelZones(init_data);
    initServicePoints(init_data);

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

            // Метка поезда для сообщений проводников в журнале
            // (пустое имя не затирает метку "поезд #N" из setTrainIndex)
            if (!train->getName().empty())
                train->getConductors().setLabel(
                            QString::fromStdString(train->getName()));

            trains.push_back(train);

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

    // Создаем коллайдеры для всех ПЕ после их расстановки на топологии
    for (auto vehicle : vehicles)
    {
        vehicle->createCollisionBodies(&collision_world);

        // Привязка питания от КС (пикетаж + ток -> состояние питания)
        catenary::CatenarySystem* cs = &catenary_system;
        vehicle->setCatenaryFeed(
            [cs](double railway_coord, double current_a) -> catenary::FeedState
        {
            return cs->getFeedState(railway_coord, current_a);
        });

        // Зоны тоннелей маршрута (ТЗ "43-47": аэродинамика "поршня")
        vehicle->getTunnel().setZones(tunnel_zones);
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

        connect(*it, &Vehicle::sigGetTrainParams, this, &Model::slotGetTrainParams);

        ++i;
    }
    tcp_server->setVehiclesInfo(vehicles_info.serialize());
    Journal::instance()->info("Ready vehicles info for server");

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
const std::vector<SoundEvent>& Model::getSoundEvents() const
{
    return sound_events;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Model::getWeatherVisibility() const
{
    return weather_system.getState().visibility;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Model::getWeatherFogDensity() const
{
    return weather_system.getState().fog_density;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
unsigned Model::getDeliveredCargoCount() const
{
    unsigned count = 0;

    for (const auto vehicle : vehicles)
    {
        if (vehicle != nullptr)
        {
            count += vehicle->getCargo().getDeliveredCargoCount();
        }
    }

    return count;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Model::getDeliveredTonnes() const
{
    double tonnes = 0.0;

    for (const auto vehicle : vehicles)
    {
        if (vehicle != nullptr)
        {
            tonnes += vehicle->getCargo().getDeliveredTonnes();
        }
    }

    return tonnes;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Model::isTrainDepartureAllowed(size_t train_idx) const
{
    if (train_idx >= trains.size())
        return true;

    // Готовность проводников поезда (система выключена - true)
    return trains[train_idx]->isDepartureAllowed();
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
void Model::slotSetVehicleControlCommand(int vehicle_idx, int cab_idx, uint16_t id, float value)
{
    for (auto *train : trains)
    {
        for (auto *vehicle : *(train->getVehicles()))
        {
            if (vehicle_idx == vehicle->getModelIndex())
            {
                if (!vehicle->control_inputs.empty())
                {
                    if (cab_idx >= 0 && cab_idx < vehicle->control_inputs.size())
                    {
                        vehicle->control_inputs[cab_idx][id] = value;
                    }
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

        //train->setTrainIndex(trains.size());
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
            train->setTrainIndex(trains.size());
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
void Model::initCollisionWorld(const init_data_t &init_data)
{
    Journal::instance()->info("==== Collision world loading ====");

    if (!collision_world.init())
    {
        Journal::instance()->error("FAILED COLLISION WORLD INIT!!!");
        return;
    }

    // В маршрутах RRS ось Z направлена вверх
    collision_world.setGravity(0.0f, 0.0f, -9.81f);

    FileSystem &fs = FileSystem::getInstance();
    const std::string route_dir = fs.getRouteRootDir() + "/" +
        init_data.route_dir_name.toStdString();

    collision::WorldLoadStats stats;
    std::string error;
    if (!collision::loadRouteIntoWorld(collision_world, route_dir, stats, &error))
    {
        Journal::instance()->warning(QString("Collision world: route objects not loaded (%1)")
                                         .arg(error.c_str()));
        return;
    }

    Journal::instance()->info(QString("Collision world: %1 bodies created from %2 route instances")
                                  .arg(stats.objects_created)
                                  .arg(stats.instances_total));

    if (stats.instances_none > 0)
    {
        Journal::instance()->info(QString("Collision world: %1 instances without collisions")
                                      .arg(stats.instances_none));
    }

    if (stats.instances_failed > 0)
    {
        Journal::instance()->warning(QString("Collision world: %1 instances FAILED")
                                         .arg(stats.instances_failed));
    }

    if (!stats.labels_without_collider.empty())
    {
        Journal::instance()->warning(QString("Collision world: %1 labels have no entry in colliders.conf")
                                         .arg(stats.labels_without_collider.size()));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initElectrification(const init_data_t &init_data)
{
    FileSystem &fs = FileSystem::getInstance();
    const std::string route_dir = fs.getRouteRootDir() + "/" +
        init_data.route_dir_name.toStdString();

    catenary_system.load(QString::fromStdString(route_dir));

    Journal::instance()->info("==== Route electrification loaded ====");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::stepSimulationLOD()
{
    // Поезд игрока: первый ПЕ с клиентом (упрощённо - первый поезд).
    // Критическое правило: он всегда L0 (в классификаторе)
    Vehicle* player_vehicle = vehicles.empty() ? nullptr : vehicles.front();

    const double player_coord = (player_vehicle != nullptr)
            ? player_vehicle->getProfilePoint()->railway_coord
            : 0.0;

    for (auto vehicle : vehicles)
    {
        perf::TrainActivity activity;

        activity.model_idx = static_cast<unsigned>(vehicle->getModelIndex());
        activity.is_player_train = (vehicle == player_vehicle);
        activity.is_moving = std::abs(vehicle->getVelocity()) > 0.05;

        const double coord = vehicle->getProfilePoint()->railway_coord;
        activity.distance_to_player = std::abs(coord - player_coord);

        // classify() с гистерезисом (ТЗ "Оптимизация", п.2-5): понижение
        // уровня - сразу при выходе за радиус, повышение - только с
        // запасом hysteresis метров внутри границы
        vehicle->setSimulationLOD(lod_manager.classify(activity));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initLoadingPoints(const init_data_t& init_data)
{
    FileSystem &fs = FileSystem::getInstance();
    const std::string route_dir = fs.getRouteRootDir() + "/" +
        init_data.route_dir_name.toStdString();

    CfgReader cfg;

    if (!cfg.load(QString::fromStdString(route_dir) + "/loading.conf"))
    {
        // Маршрут без промышленности - не ошибка
        return;
    }

    auto node = cfg.getFirstSection("LoadingPoint");

    while (!node.isNull())
    {
        LoadingPoint point;

        cfg.getString(node, "Trajectory", point.trajectory);
        cfg.getDouble(node, "Begin", point.begin);
        cfg.getDouble(node, "End", point.end);
        cfg.getString(node, "Cargo", point.cargo);
        cfg.getDouble(node, "Rate", point.rate);
        cfg.getBool(node, "Unloading", point.unloading);
        cfg.getBool(node, "Automatic", point.automatic);

        // Пассажирские остановки (ТЗ "Система проводников"):
        // очередь посадки и высота платформы над головкой рельса
        cfg.getInt(node, "Queue", point.queue);
        cfg.getDouble(node, "PlatformHeight", point.platform_height);
        point.platform_height = std::max(point.platform_height, 0.0);

        // Заказ точки разгрузки (ТЗ "Погрузка", п.18 - экономика):
        // нужный груз и количество тонн
        cfg.getString(node, "OrderCargo", point.order_cargo);
        cfg.getDouble(node, "OrderAmount", point.order_amount);
        point.order_amount = std::max(point.order_amount, 0.0);

        if (!point.trajectory.isEmpty() && point.end > point.begin)
        {
            loading_points.push_back(point);
        }

        node = cfg.getNextSection();
    }

    Journal::instance()->info(QString("Loading points loaded: %1")
                              .arg(loading_points.size()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initTunnelZones(const init_data_t& init_data)
{
    FileSystem &fs = FileSystem::getInstance();
    const std::string route_dir = fs.getRouteRootDir() + "/" +
        init_data.route_dir_name.toStdString();

    CfgReader cfg;

    if (!cfg.load(QString::fromStdString(route_dir) + "/tunnel.conf"))
    {
        // Маршрут без тоннелей - не ошибка
        return;
    }

    auto node = cfg.getFirstSection("Tunnel");

    while (!node.isNull())
    {
        double begin = 0.0;
        double end = 0.0;
        cfg.getDouble(node, "Begin", begin);
        cfg.getDouble(node, "End", end);

        if (end > begin)
        {
            tunnel_zones.emplace_back(begin, end);
        }

        node = cfg.getNextSection();
    }

    Journal::instance()->info(QString("Tunnel zones loaded: %1")
                              .arg(tunnel_zones.size()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initServicePoints(const init_data_t& init_data)
{
    FileSystem &fs = FileSystem::getInstance();
    const std::string route_dir = fs.getRouteRootDir() + "/" +
        init_data.route_dir_name.toStdString();

    CfgReader cfg;

    if (!cfg.load(QString::fromStdString(route_dir) + "/service.conf"))
    {
        // Маршрут без колонок снабжения - не ошибка
        return;
    }

    auto node = cfg.getFirstSection("ServicePoint");

    while (!node.isNull())
    {
        ServiceZone zone;
        cfg.getDouble(node, "Begin", zone.begin);
        cfg.getDouble(node, "End", zone.end);

        QString resources = "";
        cfg.getString(node, "Resources", resources);

        const QString res = resources.toLower();
        zone.fuel = res.isEmpty() || res.contains("fuel");
        zone.oil = res.isEmpty() || res.contains("oil");
        zone.coolant = res.isEmpty() || res.contains("coolant");
        zone.sand = res.isEmpty() || res.contains("sand");

        if (zone.end > zone.begin)
        {
            service_zones.push_back(zone);
        }

        node = cfg.getNextSection();
    }

    Journal::instance()->info(QString("Service zones loaded: %1")
                              .arg(service_zones.size()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::stepServiceZones()
{
    if (service_zones.empty())
        return;

    for (auto vehicle : vehicles)
    {
        const double coord = vehicle->getProfilePoint()->railway_coord;
        const bool standing = std::abs(vehicle->getVelocity()) < 0.3;

        bool in_zone = false;

        for (const auto& zone : service_zones)
        {
            if (coord >= zone.begin && coord <= zone.end)
            {
                in_zone = true;
                break;
            }
        }

        // Подключение колонки возможно только на стоянке внутри зоны
        vehicle->getService().setInZone(in_zone && standing);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::stepLoadingOperations(double dt)
{
    if (loading_points.empty())
        return;

    for (auto vehicle : vehicles)
    {
        auto& vc = topology->getVehicleController(vehicle->getModelIndex());

        const QString traj_name = vc.getCurrentTrajectoryName();

        if (traj_name.isEmpty())
            continue;

        const double coord = vehicle->getProfilePoint()->railway_coord;

        for (const LoadingPoint& point : loading_points)
        {
            if (traj_name != point.trajectory)
                continue;

            // Вагон в зоне точки (позиционирование, ТЗ п.8)
            if (coord < point.begin || coord > point.end)
                continue;

            // Погрузка/разгрузка только на стоянке
            if (std::abs(vehicle->getVelocity()) > 0.3)
                continue;

            auto& cargo = vehicle->getCargo();

            if (point.automatic)
            {
                if (point.unloading)
                {
                    if (cargo.getState() != CargoSystem::State::Unloading)
                        cargo.startUnloading();
                }
                else
                {
                    cargo.startLoading(point.cargo);
                }
            }

            // Экономика доставки (ТЗ п.18): запоминаем груз до шага,
            // чтобы завершённую разгрузку можно опознать по опустевшему
            // вагону (step() очищает тип груза при полном опустошении)
            const QString cargo_type_before = cargo.getCargoType();
            const double cargo_mass_before = cargo.getCargoMass();

            // Шаг операции со скоростью оборудования точки
            cargo.step(dt, point.rate);

            // Завершённая разгрузка в точке с заказом - засчитываем
            // доставку (без UI-экономики: только данные и журнал)
            if (point.unloading &&
                (cargo_mass_before > 0.0) &&
                (cargo.getState() == CargoSystem::State::Empty) &&
                (point.order_amount > 0.0))
            {
                // Заказ принимает только заказанный груз (пустой
                // OrderCargo - любой груз точки)
                const bool cargo_match = point.order_cargo.isEmpty() ||
                                         (point.order_cargo == cargo_type_before);

                if (cargo_match)
                {
                    const double tonnes = cargo_mass_before / 1000.0;

                    cargo.markDelivered(cargo_type_before, tonnes);

                    Journal::instance()->info(QString(
                        "[CARGO] Order delivered: '%1' %2 t at point %3-%4 m "
                        "(vehicle #%5, total: %6 deliveries / %7 t)")
                        .arg(cargo_type_before)
                        .arg(tonnes, 0, 'f', 1)
                        .arg(point.begin, 0, 'f', 0)
                        .arg(point.end, 0, 'f', 0)
                        .arg(static_cast<int>(vehicle->getModelIndex()))
                        .arg(cargo.getDeliveredCargoCount())
                        .arg(cargo.getDeliveredTonnes(), 0, 'f', 1));
                }
            }
        }
    }

    //--- Проводники пассажирских вагонов (ТЗ "Система проводников") ---
    // Цикл WAITING_FOR_TRAIN -> ... -> DESPAWN запускается для
    // стоящего в зоне пассажирского поезда; двери и потоки пассажиров
    // управляются по фазам проводников. Контекст задаётся в потоке
    // модели ДО выдачи шага поездам (process() не начинает новый тик,
    // пока все поезда не завершили предыдущий), поэтому доступа
    // проводников из двух потоков одновременно не бывает

    // Поезд игрока: для оптимизации NPC ACTIVE/INACTIVE (п.19 ТЗ)
    Vehicle* player_vehicle = vehicles.empty() ? nullptr : vehicles.front();

    const double player_coord = (player_vehicle != nullptr)
            ? player_vehicle->getProfilePoint()->railway_coord
            : 0.0;

    for (Train* train : trains)
    {
        auto& conductors = train->getConductors();

        if (!conductors.isEnabled() || !conductors.hasConductors())
            continue;

        const bool stopped = std::abs(train->getVelocity()) <= 0.3;

        // Зона станции: любой пассажирский вагон состава стоит в зоне
        // точки погрузки (платформа общая для всей остановки)
        bool in_zone = false;
        double platform_height = 1.1;
        int queue = -1;

        if (stopped)
        {
            for (auto vehicle : *(train->getVehicles()))
            {
                if (!vehicle->getPassengers().isConfigured())
                    continue;

                auto& vc = topology->getVehicleController(
                            vehicle->getModelIndex());

                const QString traj_name = vc.getCurrentTrajectoryName();

                if (traj_name.isEmpty())
                    continue;

                const double coord =
                        vehicle->getProfilePoint()->railway_coord;

                for (const LoadingPoint& point : loading_points)
                {
                    if (traj_name != point.trajectory)
                        continue;

                    if (coord >= point.begin && coord <= point.end)
                    {
                        in_zone = true;
                        platform_height = point.platform_height;
                        queue = point.queue;
                        break;
                    }
                }

                if (in_zone)
                    break;
            }
        }

        conductors.setStationZone(in_zone);
        conductors.setBoardingActive(in_zone && stopped);

        // Близость к игроку: любой вагон состава ближе 300 м
        bool player_near = false;

        for (auto vehicle : *(train->getVehicles()))
        {
            if (std::abs(vehicle->getProfilePoint()->railway_coord -
                         player_coord) <= 300.0)
            {
                player_near = true;
                break;
            }
        }

        conductors.setPlayerNear(player_near);

        // Обслуживание вагонов: платформа, потоки, двери
        for (auto vehicle : *(train->getVehicles()))
        {
            conductor::Conductor* cond = conductors.getConductor(
                        vehicle->getModelIndex());

            if (cond == nullptr)
                continue;

            auto& passengers = vehicle->getPassengers();

            // Платформа: высота поверхности над головкой рельса
            // (тип определяет проводник по разности высот, п.5 ТЗ)
            conductor::PlatformInfo platform;
            platform.known = in_zone;
            platform.height_above_rail = platform_height;
            platform.side = 1;
            cond->setPlatform(platform);

            // Состояние потоков (п.16 ТЗ): дверь не закрывается,
            // пока идёт высадка/посадка
            cond->setPassengerState(passengers.isBoardingInProgress(),
                                    passengers.isAlightingInProgress());

            // Двери вагона открывает/закрывает проводник
            passengers.setDoorsOpen(
                        cond->getDoorState() == conductor::DoorState::OPEN);

            // Высадка на прибытии: выходят все пассажиры вагона
            if (cond->fetchBeginAlightingRequest())
            {
                const int exiting = passengers.getPassengerCount();
                passengers.beginAlighting(exiting);
                cond->markAlightingDelivered(exiting);
            }

            // Посадка после высадки: очередь из конфига зоны
            // (ключ Queue, дефолт - половина вместимости)
            int count = 0;

            if (cond->fetchBeginBoardingRequest(count))
            {
                const int waiting = (count > 0) ? count
                        : ((queue >= 0) ? queue
                                        : passengers.getCapacity() / 2);

                passengers.beginBoarding(waiting);
                cond->markBoardingDelivered(waiting);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::processCollisionEvents()
{
    collision::CollisionEvent event;

    collision_active_vehicles.clear();

    while (collision_world.pollEvent(event))
    {
        if (event.type != collision::EventType::ContactAdded &&
            event.type != collision::EventType::ContactPersisted)
        {
            continue;
        }

        // user_data != nullptr только у тел ПЕ (см. Vehicle::createCollisionBodies)
        Vehicle* vehicle_a = static_cast<Vehicle*>(event.user_data_a);
        Vehicle* vehicle_b = static_cast<Vehicle*>(event.user_data_b);

        // Столкновение ПЕ-ПЕ (P1-4 аудита): обе стороны - подвижной состав.
        // Продольную силу удара создают сцепки - здесь только
        // повреждения, звук и журнал, без дублирования силы
        if ((vehicle_a != nullptr) && (vehicle_b != nullptr))
        {
            // Удерживаем аварийное состояние, пока контакт продолжается
            collision_active_vehicles.insert(vehicle_a);
            collision_active_vehicles.insert(vehicle_b);

            if (event.type == collision::EventType::ContactAdded)
            {
                // Относительная скорость ПЕ вдоль нормали контакта:
                // скорость ПЕ направлена вдоль орты её траектории
                const profile_point_t* pp_a = vehicle_a->getProfilePoint();
                const profile_point_t* pp_b = vehicle_b->getProfilePoint();

                const double va = vehicle_a->getVelocity() *
                        (pp_a->orth.x * event.normal.x +
                         pp_a->orth.y * event.normal.y +
                         pp_a->orth.z * event.normal.z);
                const double vb = vehicle_b->getVelocity() *
                        (pp_b->orth.x * event.normal.x +
                         pp_b->orth.y * event.normal.y +
                         pp_b->orth.z * event.normal.z);

                const double rel_speed = std::abs(va - vb);

                // Энергия удара по приведённой массе пары
                const double mass_a = vehicle_a->getMass();
                const double mass_b = vehicle_b->getMass();
                const double reduced_mass = (mass_a + mass_b > 0.0)
                        ? mass_a * mass_b / (mass_a + mass_b)
                        : 0.0;
                const double energy = 0.5 * reduced_mass *
                        rel_speed * rel_speed;

                // Повреждения каждой ПЕ от её доли энергии удара
                vehicle_a->onCollisionContact(event);
                vehicle_b->onCollisionContact(event);

                Journal::instance()->critical(QString(
                    "[COLLISION] Vehicle #%1 vs vehicle #%2: "
                    "rel.speed %3 km/h, energy %4 MJ")
                    .arg(static_cast<int>(vehicle_a->getModelIndex()))
                    .arg(static_cast<int>(vehicle_b->getModelIndex()))
                    .arg(rel_speed * 3.6, 0, 'f', 1)
                    .arg(energy / 1.0e6, 0, 'f', 2));

                // Звук удара в точке контакта (интенсивность по энергии)
                if (rel_speed > 0.5)
                {
                    SoundEvent impact;
                    impact.type = SoundEventType::CouplerImpact;
                    impact.x = event.point.x;
                    impact.y = event.point.y;
                    impact.z = event.point.z;
                    impact.intensity = std::clamp(energy / 1.0e6, 0.1, 1.0);
                    impact.rate_hz = 0.0;
                    impact.vehicle_idx = vehicle_a->getModelIndex();
                    sound_events.push_back(impact);
                }
            }

            continue;
        }

        // Оба - мир: не интересует
        if ((vehicle_a != nullptr) == (vehicle_b != nullptr))
            continue;

        // Контакт ПЕ с миром
        Vehicle* vehicle = (vehicle_a != nullptr) ? vehicle_a : vehicle_b;
        const collision::Layer other_layer =
            (vehicle_a != nullptr) ? event.layer_b : event.layer_a;

        switch (other_layer)
        {
        // Препятствия, при контакте с которыми ПЕ аварийно тормозит
        case collision::Layer::Infrastructure:
        case collision::Layer::Terrain:
        case collision::Layer::Track:
            collision_active_vehicles.insert(vehicle);
            if (event.type == collision::EventType::ContactAdded)
            {
                ++collision_contacts_total;
                vehicle->onCollisionContact(event);
            }
            break;

        default:
            break;
        }
    }

    // Снимаем аварийное состояние с ПЕ, чей контакт прекратился
    for (auto vehicle : vehicles)
    {
        if (vehicle->isCollided() &&
            collision_active_vehicles.count(vehicle) == 0)
        {
            vehicle->resetCollisionState();
        }
    }

    // Повреждение пути при сходе ПЕ (ТЗ "Реалистичный сход ПС", п.27):
    // неровности траектории усиливаются, влияя на следующие поезда
    for (auto vehicle : vehicles)
    {
        if (vehicle->isDerailed() &&
            derailed_vehicles.count(vehicle) == 0)
        {
            derailed_vehicles.insert(vehicle);

            topology->getVehicleController(vehicle->getModelIndex())
                .damageTrack(0.5);

            Journal::instance()->critical(QString(
                "[DERAILMENT] Vehicle #%1 damaged the track "
                "(irregularities increased)")
                .arg(static_cast<int>(vehicle->getModelIndex())));

            // Цепной сход (ТЗ "Физика после схода", п.21): сошедшая ПЕ
            // резко тормозит и дёргает соседей через сцепки - боковой
            // импульс разгружает их колёса
            const double chain_energy = 0.05 * vehicle->getMass() *
                    vehicle->getVelocity() * vehicle->getVelocity();

            Vehicle* neighbors[2] = {vehicle->getPrevVehicle(),
                                     vehicle->getNextVehicle()};

            for (Vehicle* neighbor : neighbors)
            {
                if (neighbor != nullptr && !neighbor->isDerailed())
                {
                    neighbor->onCouplerJerk(chain_energy);
                }
            }
        }
    }

    // Сводка в журнал примерно раз в 10 с модельного времени
    constexpr std::uint64_t summary_period = 667;   // ~10 с при шаге 15 мс
    if (++collision_events_step >= summary_period)
    {
        collision_events_step = 0;

        if (collision_contacts_total > 0)
        {
            Journal::instance()->info(QString("Collision world: %1 contacts total")
                                          .arg(collision_contacts_total));
        }
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

    connect(topology, &Topology::sendTrajBusyState, tcp_server, &TcpServer::slotSendTrajBusyState);

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

    connect(tcp_server, &TcpServer::sigSetSimSpeed, this, &Model::slotSetSimSpeed);

    connect(tcp_server, &TcpServer::sigSetVehicleControlCommand, this, &Model::slotSetVehicleControlCommand);

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
    update_pos_data.vehicles.resize(vehicles.size());
    update_vehicles.vehicles.resize(vehicles.size());
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

        // Реакция камеры от физики (ТЗ "Физическая реакция машиниста"):
        // смещение/наклон головы машиниста для кабельной камеры клиента
        const auto& cam_motion = vehicle->getCameraMotion();
        update_pos_data.vehicles[i].cam_offset_x =
                static_cast<float>(cam_motion.getOffsetX());
        update_pos_data.vehicles[i].cam_offset_y =
                static_cast<float>(cam_motion.getOffsetY());
        update_pos_data.vehicles[i].cam_offset_z =
                static_cast<float>(cam_motion.getOffsetZ());
        update_pos_data.vehicles[i].cam_tilt_roll =
                static_cast<float>(cam_motion.getTiltRoll());
        update_pos_data.vehicles[i].cam_tilt_pitch =
                static_cast<float>(cam_motion.getTiltPitch());

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

    // Погода для рендера клиента (ТЗ "Видимость и погода"): дальняя
    // плоскость камеры и туман едут вместе с позициями ПЕ
    {
        const auto& wstate = weather_system.getState();
        update_pos_data.visibility_m = static_cast<float>(wstate.visibility);
        update_pos_data.fog_density = static_cast<float>(wstate.fog_density);
    }

    // Звуковые события последнего шага физики (ТЗ "Аудиосистема"):
    // раздаются клиентам вместе с позициями (события несут мировые
    // координаты, пул источников собирает клиент)
    update_pos_data.sound_events.clear();
    update_pos_data.sound_events.reserve(sound_events.size());

    for (const auto& event : sound_events)
    {
        simulator_sound_event_t net_event;
        net_event.type = static_cast<quint8>(event.type);
        net_event.x = static_cast<float>(event.x);
        net_event.y = static_cast<float>(event.y);
        net_event.z = static_cast<float>(event.z);
        net_event.intensity = static_cast<float>(event.intensity);
        net_event.rate_hz = static_cast<float>(event.rate_hz);
        net_event.vehicle_idx = static_cast<quint32>(event.vehicle_idx);

        update_pos_data.sound_events.push_back(net_event);
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
// Снимок диагностики составов (ТЗ "Промт статистики вагонов", F3/F4):
// по каждому вагону - масса/скорость/силы/повреждения, по составу -
// сводка продольной динамики. Вызывается из потока модели между
// шагами поездов (как prepareFeedBack)
//------------------------------------------------------------------------------
void Model::prepareDiagnostics()
{
    update_diagnostics.trains.clear();
    update_diagnostics.vehicles.clear();

    update_diagnostics.trains.reserve(trains.size());
    update_diagnostics.vehicles.reserve(vehicles.size());

    for (auto train : trains)
    {
        if ((train == nullptr) || (train->getVehicles() == nullptr) ||
            (train->getFirstVehicle() == nullptr) ||
            (train->getLastVehicle() == nullptr))
        {
            continue;
        }

        simulator_train_diagnostics_t train_diag;

        train_diag.first_vehicle_id = train->getFirstVehicle()->getModelIndex();
        train_diag.last_vehicle_id = train->getLastVehicle()->getModelIndex();
        train_diag.train_name = QString(train->getName().c_str());

        const auto stats = train->getLongitudinalStats();
        train_diag.train_mass_t = static_cast<float>(stats.train_mass / 1000.0);
        train_diag.train_length_m = static_cast<float>(stats.train_length);
        train_diag.max_tension_kn = static_cast<float>(stats.max_tension / 1000.0);
        train_diag.max_compression_kn = static_cast<float>(stats.max_compression / 1000.0);
        train_diag.max_abs_force_kn = static_cast<float>(stats.max_abs_force / 1000.0);
        train_diag.overloaded_joints = stats.overloaded_joints;
        train_diag.broken_joints = stats.broken_joints;

        update_diagnostics.trains.push_back(train_diag);

        for (const auto& vd : train->buildDiagnosticsSnapshot())
        {
            simulator_vehicle_diagnostics_t diag;
            diag.vehicle_idx = static_cast<int>(vd.vehicle_idx);
            diag.mass_t = static_cast<float>(vd.mass_kg / 1000.0);
            diag.speed_kmh = static_cast<float>(vd.speed_kmh);
            diag.force_kn = static_cast<float>(vd.longitudinal_force_n / 1000.0);
            diag.vertical_accel = static_cast<float>(vd.vertical_accel);
            diag.lateral_accel = static_cast<float>(vd.lateral_accel);
            diag.body_damage = static_cast<float>(vd.body_damage);
            diag.bogie_damage = static_cast<float>(vd.bogie_damage);
            diag.brake_efficiency = static_cast<float>(vd.brake_efficiency);
            diag.shoe_temperature = static_cast<float>(vd.shoe_temperature);
            diag.rail_coord_m = static_cast<float>(vd.rail_coord_m);
            diag.inclination = static_cast<float>(vd.inclination);
            diag.derailed = vd.derailed ? 1 : 0;
            diag.coupled_fwd = vd.coupled_fwd ? 1 : 0;
            diag.coupled_bwd = vd.coupled_bwd ? 1 : 0;

            update_diagnostics.vehicles.push_back(diag);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::tcpFeedBack(bool need_trains_feedback)
{
    double realtime_seconds = std::chrono::duration<double, std::chrono::seconds::period>(process_timepoint - start_timepoint).count();

    if (need_trains_feedback)
    {
        tcp_server->updateTrainsInfo(update_trains.serialize());
        update_trains = simulator_trains_update_t();
    }

    // Диагностика - компактный снимок не чаще раза в 0.5 с (ТЗ F3/F4)
    if ((realtime_seconds - diagnostics_prev_send_time) >= 0.5)
    {
        diagnostics_prev_send_time = realtime_seconds;
        prepareDiagnostics();
        tcp_server->updateDiagnostics(update_diagnostics.serialize(),
                                      realtime_seconds);
    }

    tcp_server->updateVehiclesPos(update_pos_data.serialize(), realtime_seconds);
    update_pos_data = simulator_update_pos_t();

    tcp_server->updateVehiclesState(update_vehicles.serialize(), realtime_seconds);
    update_vehicles = simulator_vehicles_update_t();

    tcp_server->updatePlayers(update_players.serialize(), realtime_seconds);
    update_players = simulator_update_players_t();

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

    // Профилировщик: начало кадра физики (ТЗ "Оптимизация")
    profiler.beginFrame();

    // Классификация LOD поездов + адаптивная производительность
    stepSimulationLOD();

    // Операции погрузки/разгрузки (ТЗ "Погрузка")
    {
        perf::ScopedTimer timer(profiler, "loading_ops");
        stepLoadingOperations(integration_time);
    }

    // Отметка ПЕ в зонах заправочных колонок (ТЗ "Снабжение")
    stepServiceZones();

    {
        perf::ScopedTimer timer(profiler, "weather");
        weather_system.step(static_cast<double>(integration_time));
    }

    // Завершение кадра профилировщика (после всех систем)
    profiler.endFrame();
    adaptive_perf.step(static_cast<double>(integration_time),
                       profiler, lod_manager);

    // Звуковые события от физики ПЕ (ТЗ "Аудиосистема")
    sound_events.clear();

    for (auto vehicle : vehicles)
    {
        vehicle->collectSoundEvents(sound_events);
    }

    // Погода: раздача на системы (ТЗ "Видимость и погода"):
    // сцепление колёс (осадки) и токоприёмники (ветер)
    {
        const auto& wstate = weather_system.getState();

        for (auto vehicle : vehicles)
        {
            vehicle->getAdhesion().setWeather(
                        static_cast<WheelRailAdhesion::Weather>(
                            static_cast<int>(wstate.type)),
                        wstate.intensity);
            vehicle->getAdhesion().setTemperature(wstate.temperature);
            vehicle->applyWindToPantograph(wstate.wind_speed);
            vehicle->setRainIntensity(wstate.intensity);
        }
    }

    // Баланс энергии секций КС (ТЗ "Рекуперация", п.6-7): рекуперация
    // принимается подстанцией (если обратимая) и потребителями той же
    // секции; остаток идёт в реостаты ПЕ
    {
        std::map<QString, double> section_load;

        for (auto vehicle : vehicles)
        {
            // Отрыв полоза (дуга) - ПЕ не питается от КС и не может
            // отдавать энергию: контакт токоприёмника обязателен
            if (!vehicle->getPantograph().isRaised() ||
                !vehicle->getPantograph().isContactOk() ||
                !vehicle->getCatenaryFeedActive())
            {
                continue;
            }

            const double coord = vehicle->getProfilePoint()->railway_coord;
            const auto feed = catenary_system.getFeedState(coord, 0.0);

            if (!feed.powered)
                continue;

            const QString key = feed.substation_id;

            const double load_kw =
                    vehicle->getEnergy().getPower();

            if (load_kw > 0.0)
                section_load[key] += load_kw * 1000.0;
        }

        for (auto vehicle : vehicles)
        {
            // Без устойчивого контакта приём рекуперации невозможен:
            // энергия уходит в реостаты ПЕ, а не "возвращается" в КС
            // при отрыве полоза
            if (!vehicle->getPantograph().isRaised() ||
                !vehicle->getPantograph().isContactOk() ||
                !vehicle->getCatenaryFeedActive())
            {
                vehicle->setRegenAcceptance(0.0, false);
                continue;
            }

            const double coord = vehicle->getProfilePoint()->railway_coord;
            const auto feed = catenary_system.getFeedState(coord, 0.0);

            if (!feed.powered || !feed.accepts_regen)
            {
                vehicle->setRegenAcceptance(0.0, false);
                continue;
            }

            // Приём: подстанция + потребители этой секции
            const QString key = feed.substation_id;
            const double local_load = section_load.count(key) > 0
                    ? section_load.at(key) : 0.0;

            vehicle->setRegenAcceptance(
                        feed.max_regen_power + local_load, true);
        }
    }

    // Шаг мира коллизий и разбор его событий.
    // Jolt стабилен при подшаге <= 1/60 с: дробим интервал на подшаги
    const int collision_steps = std::max(1, static_cast<int>(
        std::ceil(integration_time / (1.0 / 60.0))));
    {
        perf::ScopedTimer timer(profiler, "collision");
        collision_world.step(static_cast<float>(integration_time),
                             collision_steps);
    }
    processCollisionEvents();

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

    // Шаг всех поездов: поезда живут в своих потоках, шаг выдаётся
    // сигналом ниже, завершение фиксируется slotTrainStepDone. Замер
    // секции physics_trains (ТЗ "Оптимизация") - честный полный цикл
    // диспетчеризации: от выдачи шага до завершения последнего поезда
    // (фиксация в slotTrainStepDone, Profiler потокобезопасен)
    trains_step_start_ = std::chrono::steady_clock::now();

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

        // Замер секции шага поездов (ТЗ "Оптимизация"): полный цикл
        // диспетчеризации - от выдачи step (process) до завершения
        // шага последнего поезда, включая ожидание в потоке модели
        const std::chrono::duration<double, std::milli> trains_step_ms =
                end_timepoint - trains_step_start_;
        profiler.record("physics_trains", trains_step_ms.count());
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
        Journal::instance()->error(QString("Rename train: Train index out of range (%1)").arg(t_idx, 4));
        return;
    }

    trains[t_idx]->setName(new_name.toStdString());

    Journal::instance()->info(QString("Rename train: Train %1 has new name %2").arg(t_idx, 4).arg(new_name));

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
