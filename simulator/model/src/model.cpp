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
    simulator_info_t tmp_si = simulator_info_t();
    memory_sim_info.setKey(SHARED_MEMORY_SIM_INFO);
    if (memory_sim_info.create(sizeof(simulator_info_t)))
    {
        Journal::instance()->info("Created shared memory for simulator info");
        memcpy(memory_sim_info.data(), &tmp_si, sizeof (simulator_info_t));
    }
    else
    {
        if (memory_sim_info.attach())
        {
            Journal::instance()->info("Attach to shared memory for simulator info");
            memcpy(memory_sim_info.data(), &tmp_si, sizeof (simulator_info_t));
        }
        else
        {
            Journal::instance()->error("No shared memory for simulator info");
        }
    }

    memory_sim_update.setKey(SHARED_MEMORY_SIM_UPDATE);
    if (memory_sim_update.create(sizeof(simulator_update_t)))
    {
        Journal::instance()->info("Created shared memory for simulator update data");
    }
    else
    {
        if (memory_sim_update.attach())
        {
            Journal::instance()->info("Attach to shared memory for simulator update data");
        }
        else
        {
            Journal::instance()->error("No shared memory for simulator update data");
        }
    }

    controlled_t tmp_c = controlled_t();
    memory_controlled.setKey(SHARED_MEMORY_CONTROLLED);
    if (memory_controlled.create(sizeof(controlled_t)))
    {
        Journal::instance()->info("Created shared memory for info about controlled vehicle");
        memcpy(memory_controlled.data(), &tmp_c, sizeof (controlled_t));
    }
    else
    {
        if (memory_controlled.attach())
        {
            Journal::instance()->info("Attach to shared memory for info about controlled vehicle");
            memcpy(memory_controlled.data(), &tmp_c, sizeof (controlled_t));
        }
        else
        {
            Journal::instance()->error("No shared memory for info about controlled vehicle");
        }
    }

    keys_data.setKey(SHARED_MEMORY_KEYS_DATA);
    if (keys_data.create(sizeof(KEYS_DATA_BYTEARRAY_SIZE)))
    {
        Journal::instance()->info("Created shared memory for keysboard processing");
    }
    else
    {
        if (keys_data.attach())
        {
            Journal::instance()->info("Attach to shared memory for keysboard processing");
        }
        else
        {
            Journal::instance()->error("No shared memory for keyboard data. Unable process keyboard");
        }
    }    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Model::~Model()
{
    memory_sim_info.detach();
    memory_sim_update.detach();
    memory_controlled.detach();
    keys_data.detach();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Model::init(const simulator_command_line_t &command_line)
{
    // Check is debug print allowed
    is_debug_print = command_line.debug_print.is_present;

    init_data_t init_data;

    // Load initial data configuration
    Journal::instance()->info("==== Init data loading ====");
    loadInitData(init_data);

    // Override init data by command line
    Journal::instance()->info("==== Command line processing ====");
    overrideByCommandLine(init_data, command_line);

    // Read solver configuration
    Journal::instance()->info("==== Solver configurating ====");
    configSolver(init_data.solver_config);

    start_time = init_data.solver_config.start_time;
    stop_time = init_data.solver_config.stop_time;
    dt = init_data.solver_config.step;
    integration_time_interval = init_data.integration_time_interval;

    // Load profile
    Journal::instance()->info("==== Profile data loading ====");
    FileSystem &fs = FileSystem::getInstance();
    std::string route_dir_path = fs.combinePath(fs.getRouteRootDir(), init_data.route_dir_name.toStdString());
    profile = new Profile(init_data.direction, route_dir_path);

    Journal::instance()->info(QString("State Profile object at address: 0x%1")
                              .arg(reinterpret_cast<quint64>(profile), 0, 16));

    if (profile->isReady())
    {
        Journal::instance()->info("Profile loaded successfully");
    }
    else
    {
        Journal::instance()->warning("Profile is't loaded. Using flat profile");
    }

    // Train creation and initialization
    Journal::instance()->info("==== Train initialization ====");
    train = new Train(profile);
    train->setTopology(topology);

    Journal::instance()->info(QString("Created Train object at address: 0x%1")
                              .arg(reinterpret_cast<quint64>(train), 0, 16));

    if (!train->init(init_data))
        return false;

    Journal::instance()->info("==== Info to shared memory ====");
    simulator_info_t   info_data;
    info_data.num_updates = 1;
    info_data.route_info.route_dir_name_length = init_data.route_dir_name.size();
    init_data.route_dir_name.toWCharArray(info_data.route_info.route_dir_name);
    Journal::instance()->info("Ready route info for shared memory");

    std::vector<Vehicle *> *vehicles = train->getVehicles();
    info_data.num_vehicles = vehicles->size();
    size_t i = 0;
    for (auto it = vehicles->begin(); it != vehicles->end(); ++it)
    {
        QString dir = (*it)->getConfigDir();
        info_data.vehicles_info[i].vehicle_config_dir_length = dir.size();
        dir.toWCharArray(info_data.vehicles_info[i].vehicle_config_dir);

        QString file = (*it)->getConfigName();
        info_data.vehicles_info[i].vehicle_config_file_length = file.size();
        file.toWCharArray(info_data.vehicles_info[i].vehicle_config_file);

        ++i;
    }
    Journal::instance()->info("Ready vehicles info for shared memory");

    if (memory_sim_info.lock())
    {
        memcpy(memory_sim_info.data(), &info_data, sizeof (simulator_info_t));
        memory_sim_info.unlock();
        Journal::instance()->info("Set info to shared memory");
    }
    else
    {
        Journal::instance()->error("Can't lock shared memory");
    }

    initControlPanel("control-panel");

    initSimClient("virtual-railway");

    //initSignaling(init_data);

    //initTraffic(init_data);

    /*for (Vehicle *vehicle : *(train->getVehicles()))
    {
        connect(vehicle, &Vehicle::sendCoord,
                signaling, &Signaling::set_busy_sections);
    }*/

    initTopology(init_data);

    initTcpServer();

    Journal::instance()->info("Train is initialized successfully");

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
        t = start_time;

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
void Model::controlProcess()
{
    control_panel->process();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::preStep(double t)
{
    train->preStep(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Model::step(double t, double &dt)
{
    if (!train->step(t, dt))
        return false;

    //signaling->step(t, dt);

    /*double coord = train->getVehicles()->at(0)->getRailwayCoord() +
            train->getDirection() * train->getVehicles()->at(0)->getLength() / 2.0;*/

    //alsn_info_t alsn_info = signaling->getALSN(coord);
    //train->getVehicles()->at(0)->setASLN(alsn_info);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::postStep(double t)
{
    train->postStep(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::debugPrint()
{
    QString debug_info = QString("t = %1 realtime_delay = %2 time_step = %3 x = %10 v[first] = %4 v[last] = %5 trac = %6 pos = %7 eq_press = %8 bp_press = %9 pos = %11\n")
            .arg(t)
            .arg(realtime_delay)
            .arg(dt)
            .arg(train->getFirstVehicle()->getVelocity() * 3.6)
            .arg(train->getLastVehicle()->getVelocity() * 3.6)
            .arg(static_cast<double>(train->getFirstVehicle()->getAnalogSignal(0)))
            .arg(static_cast<int>(train->getFirstVehicle()->getAnalogSignal(3)))
            .arg(static_cast<double>(train->getFirstVehicle()->getAnalogSignal(2)))
            .arg(static_cast<double>(train->getFirstVehicle()->getAnalogSignal(4)))
            .arg(train->getFirstVehicle()->getRailwayCoord())
            .arg(static_cast<double>(train->getFirstVehicle()->getAnalogSignal(20)));

    fputs(qPrintable(debug_info), stdout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::loadInitData(init_data_t &init_data)
{
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
            init_data.trajectory_name = "route1_0001";
        }

        if (!cfg.getInt(secName, "Direction", init_data.direction))
        {
            init_data.direction = 1;
        }

        if (!cfg.getDouble(secName, "InitCoord", init_data.init_coord))
        {
            init_data.init_coord = 0.0;
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

        if (!cfg.getInt(secName, "ControlTimeInterval", init_data.control_time_interval))
        {
            init_data.control_time_interval = 15;
        }

        control_delay = static_cast<double>(init_data.control_time_interval) / 1000.0;

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
    if (command_line.train_config.is_present)
    {
        init_data.train_config = command_line.train_config.value;
    }

    if (command_line.route_dir.is_present)
    {
        init_data.route_dir_name = command_line.route_dir.value;
    }

    if (command_line.debug_print.is_present)
    {
        init_data.debug_print = command_line.debug_print.value;
    }

    if (command_line.init_coord.is_present)
    {
        init_data.init_coord = command_line.init_coord.value;
    }

    if (command_line.direction.is_present)
    {
        init_data.direction = command_line.direction.value;
    }

    if (command_line.trajectory_name.is_present)
    {
        init_data.trajectory_name = command_line.trajectory_name.value;
    }

    Journal::instance()->info("Apply command line settinds");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::configSolver(solver_config_t &solver_config)
{
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

        if (!cfg.getDouble(secName, "StopTime", solver_config.stop_time))
        {
            solver_config.stop_time = 10.0;
        }
        Journal::instance()->info("Stop time: " + QString("%1").arg(solver_config.stop_time));

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
        QString module_name = "";

        if (!cfg.getString(secName, "Plugin", module_name))
            return;

        control_panel = Q_NULLPTR;
        QString module_path = QString(fs.getPluginsDir().c_str()) + fs.separator() + module_name;
        control_panel = loadInterfaceDevice(module_path);

        if (control_panel == Q_NULLPTR)
            return;

        QString config_dir = "";

        if (!cfg.getString(secName, "ConfigDir", config_dir))
            return;

        config_dir = QString(fs.toNativeSeparators(config_dir.toStdString()).c_str());

        if (!control_panel->init(QString(fs.getConfigDir().c_str()) + fs.separator() + config_dir))
            return;

        int request_interval = 0;

        if (!cfg.getInt(secName, "RequestInterval", request_interval))
            request_interval = 100;

        controlTimer.setInterval(request_interval);
        connect(&controlTimer, &QTimer::timeout, this, &Model::controlProcess);

        int v_idx = 0;

        if (!cfg.getInt(secName, "Vehicle", v_idx))
            v_idx = 0;

        Vehicle *vehicle = train->getVehicles()->at(static_cast<size_t>(v_idx));

        connect(vehicle, &Vehicle::sendFeedBackSignals,
                control_panel, &VirtualInterfaceDevice::receiveFeedback);

        connect(control_panel, &VirtualInterfaceDevice::sendControlSignals,
                vehicle, &Vehicle::getControlSignals);

        controlTimer.start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initSimClient(QString cfg_path)
{
    /*if (train->getTrainID().isEmpty())
        return;

    if (train->getClientName().isEmpty())
        return;

    CfgReader cfg;
    FileSystem &fs = FileSystem::getInstance();
    QString full_path = QString(fs.getConfigDir().c_str()) + fs.separator() + cfg_path + ".xml";

    if (cfg.load(full_path))
    {
        QString secName = "VRServer";
        tcp_config_t tcp_config;

        cfg.getString(secName, "HostAddr", tcp_config.host_addr);
        int port = 0;

        if (!cfg.getInt(secName, "Port", port))
        {
            port = 1993;
        }

        tcp_config.port = static_cast<quint16>(port);
        tcp_config.name = train->getClientName();

        sim_client = new SimTcpClient();
        connect(this, &Model::getRecvData, sim_client, &SimTcpClient::getRecvData);
        sim_client->init(tcp_config);
        sim_client->start();

        Journal::instance()->info("Started virtual railway TCP-client...");

        connect(&networkTimer, &QTimer::timeout, this, &Model::virtualRailwayFeedback);
        networkTimer.start(100);
    }
    else
    {
        Journal::instance()->error("There is no virtual railway configuration in file " + full_path);
    }*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initSignaling(const init_data_t &init_data)
{
    signaling = new Signaling;

    FileSystem &fs = FileSystem::getInstance();
    std::string route_dir_path = fs.combinePath(fs.getRouteRootDir(), init_data.route_dir_name.toStdString());

    if (!signaling->init(init_data.direction, route_dir_path.c_str()))
    {
        Journal::instance()->error("Failed signaling initialization at route " +
                                   QString(route_dir_path.c_str()));
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
    if (topology->load(init_data.route_dir_name))
    {
        Journal::instance()->info("Loaded topology for route " + init_data.route_dir_name);
    }
    else
    {
        Journal::instance()->error("FAILED TOPOLOGY LOAD!!!");
    }

    topology_pos_t tp;
    tp.traj_name = init_data.trajectory_name;
    tp.traj_coord = init_data.init_coord;
    tp.dir = init_data.direction;

    if (topology->init(tp, train->getVehicles()))
    {
        Journal::instance()->info("Topology initialized successfully");
    }
    else
    {
        Journal::instance()->critical("CAN'T INITIALIZE TOPOLOGY");
        exit(0);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::initTcpServer()
{
    FileSystem &fs = FileSystem::getInstance();
    std::string cfg_path = fs.getConfigDir() + fs.separator() + "tcp-server.xml";

    tpc_server->init(QString(cfg_path.c_str()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::tcpFeedBack()
{
    /*std::vector<Vehicle *> *vehicles = train->getVehicles();

    viewer_data.time = static_cast<float>(integration_time_interval) / 1000.0f;

    size_t i = 0;
    for (auto it = vehicles->begin(); it != vehicles->end(); ++it)
    {
        viewer_data.te[i].coord = static_cast<float>((*it)->getRailwayCoord());
        viewer_data.te[i].velocity = static_cast<float>((*it)->getVelocity());
        viewer_data.te[i].coord_end = viewer_data.te[i].coord + viewer_data.te[i].velocity * viewer_data.time;

        viewer_data.te[i].angle = static_cast<float>((*it)->getWheelAngle(0));
        viewer_data.te[i].omega = static_cast<float>((*it)->getWheelOmega(0));
        viewer_data.te[i].angle_end = viewer_data.te[i].angle + viewer_data.te[i].omega * viewer_data.time;

        (*it)->getDebugMsg().toWCharArray(viewer_data.te[i].DebugMsg);

        memcpy(viewer_data.te[i].discreteSignal,
               (*it)->getDiscreteSignals(),
               sizeof (viewer_data.te[i].discreteSignal));

        memcpy(viewer_data.te[i].analogSignal,
               (*it)->getAnalogSignals(),
               sizeof (viewer_data.te[i].analogSignal));
        ++i;
    }

    QByteArray array(sizeof(server_data_t), Qt::Uninitialized);
    memcpy(array.data(), &viewer_data, sizeof(server_data_t));
    emit sendDataToServer(array);

    viewer_data.count++;

    emit sendDataToTrain(server->getReceivedData());*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::virtualRailwayFeedback()
{
    /*if (sim_client == Q_NULLPTR)
        return;

    if (!sim_client->isConnected())
        return;

    sim_dispatcher_data_t disp_data;
    emit getRecvData(disp_data);

    alsn_info_t alsn_info;
    alsn_info.code_alsn = disp_data.code_alsn;
    alsn_info.num_free_block = disp_data.num_free_block;
    alsn_info.response_code = disp_data.response_code;
    alsn_info.signal_dist = disp_data.signal_dist;
    strcpy(alsn_info.current_time, disp_data.current_time);

    train->getFirstVehicle()->setASLN(alsn_info);

    sim_train_data_t train_data;
    strcpy(train_data.train_id, train->getTrainID().toStdString().c_str());
    train_data.direction = train->getDirection();
    train_data.coord = train->getFirstVehicle()->getRailwayCoord();
    train_data.speed = train->getFirstVehicle()->getVelocity();

    sim_client->sendTrainData(train_data);*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::sharedMemoryFeedback()
{
    update_data.time = t;
    update_data.current_vehicle = current_vehicle;
    update_data.controlled_vehicle = controlled_vehicle;

    int i = 0;
    std::vector<Vehicle *> *vehicles = train->getVehicles();

    for (auto vehicle : *vehicles)
    {
        profile_point_t *pp = vehicle->getProfilePoint();

        update_data.vehicles[i].position_x = pp->position.x;
        update_data.vehicles[i].position_y = pp->position.y;
        update_data.vehicles[i].position_z = pp->position.z;
        update_data.vehicles[i].orth_x = pp->orth.x;
        update_data.vehicles[i].orth_y = pp->orth.y;
        update_data.vehicles[i].orth_z = pp->orth.z;
        update_data.vehicles[i].up_x = pp->up.x;
        update_data.vehicles[i].up_y = pp->up.y;
        update_data.vehicles[i].up_z = pp->up.z;

        update_data.vehicles[i].orientation = vehicle->getOrientation();

        std::copy(vehicle->getAnalogSignals().begin(),
                  vehicle->getAnalogSignals().end(),
                  update_data.vehicles[i].analogSignal.begin());

        if (current_vehicle == i)
        {
            QString msg = vehicle->getDebugMsg();
            msg.resize(DEBUG_STRING_SIZE, QChar(' '));
            msg.toWCharArray(update_data.currentDebugMsg);
        }

        if (controlled_vehicle == i)
        {
            if (data.size() != 0)
                vehicle->setKeysData(data);

            QString msg = vehicle->getDebugMsg();
            msg.resize(DEBUG_STRING_SIZE, QChar(' '));
            msg.toWCharArray(update_data.controlledDebugMsg);
        }
        else
        {
            if (prev_controlled_vehicle == i)
            {
                vehicle->resetKeysData();
                prev_controlled_vehicle = controlled_vehicle;
            }
        }
        ++i;
    }

    if (memory_sim_update.lock())
    {
        memcpy(memory_sim_update.data(), &update_data, sizeof (simulator_update_t));
        memory_sim_update.unlock();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::controlStep(double &control_time, const double control_delay)
{
    if (control_time >= control_delay)
    {
        control_time = 0;

        if (memory_controlled.lock())
        {
            controlled_t *c = static_cast<controlled_t *>(memory_controlled.data());

            if (c == nullptr)
            {
                memory_controlled.unlock();
                return;
            }

            current_vehicle = c->current_vehicle;
            controlled_vehicle = c->controlled_vehicle;

            memory_controlled.unlock();
        }

        if (keys_data.lock())
        {
            data.resize(keys_data.size());
            memcpy(data.data(), keys_data.data(), static_cast<size_t>(keys_data.size()));

            keys_data.unlock();
        }
    }

    control_time += dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::process()
{
    double integration_time = static_cast<double>(integration_time_interval) / 1000.0;
    tau = tau - integration_time;

    // Integrate all ODE in train motion model
    do
    {
        preStep(t);

        controlStep(control_time, control_delay);

        //double v = 50.0 / Physics::kmh;
        //signaling->set_busy_sections(5000.0 + v * t);

        is_step_correct = step(t, dt);

        tau += dt;
        t += dt;

        postStep(t);
    }
    while ( (tau < 0.0) && is_step_correct );

    // Feedback to viewer
    sharedMemoryFeedback();

    train->inputProcess();

    // Debug print, is allowed
    if (is_debug_print)
        debugPrint();
}
