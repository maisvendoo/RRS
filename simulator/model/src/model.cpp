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

#include    "CfgReader.h"
#include    "Journal.h"
#include    "JournalFile.h"

#include    "topology.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Model::Model(QObject *parent) : QObject(parent)
  , t(0.0)
  , dt(0.001)
  , start_time(0.0)
  , stop_time(1000.0)
  , is_step_correct(true)
  , is_simulation_started(false)
  , realtime_delay(0)
  , integration_time_interval(100)
  , is_debug_print(false)
  , control_time(0)
  , control_delay(0.05)
  , train(nullptr)
  , profile(nullptr)
  , server(nullptr)
  , control_panel(nullptr)
{
    shared_memory.setKey("sim");

    if (!shared_memory.create(sizeof(server_data_t)))
    {
        shared_memory.attach();
    }    

    sim_client = Q_NULLPTR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Model::~Model()
{
    shared_memory.detach();
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
    profile = new Profile(init_data.direction, init_data.route_dir.toStdString());

    Journal::instance()->info(QString("State Profile object at address: 0x%1")
                              .arg(reinterpret_cast<quint64>(profile), 0, 16));

    if (profile->isReady())
        Journal::instance()->info("Profile loaded successfully");
    else
    {
        Journal::instance()->warning("Profile is't loaded. Using flat profile");
    }

    // Train creation and initialization
    Journal::instance()->info("==== Train initialization ====");
    train = new Train(profile);

    Journal::instance()->info(QString("Created Train object at address: 0x%1")
                              .arg(reinterpret_cast<quint64>(train), 0, 16));

    connect(train, &Train::logMessage, this, &Model::logMessage);

    if (!train->init(init_data))
        return false;    

    connect(this, &Model::sendDataToTrain, train, &Train::sendDataToVehicle);

    keys_data.setKey("keys");    

    if (!keys_data.create(init_data.keys_buffer_size))
    {
        if (!keys_data.attach())
        {
            Journal::instance()->error("Can't attach to shread memory. Unable process keyboard");
        }
    }
    else
    {
        Journal::instance()->info("Created shared memory for keysboard processing");
    }

    initControlPanel("control-panel");

    initSimClient("virtual-railway");

    Journal::instance()->info("Train is initialized successfully");


    /*topology.load(init_data.route_dir);
    topology_pos_t tp;
    tp.traj_name = "s01-chp1";
    tp.traj_coord = 700.0;
    tp.dir = 1;

    topology.init(tp, train->getVehicles());*/

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

        if (!cfg.getDouble(secName, "InitCoord", init_data.init_coord))
        {
            init_data.init_coord = 1.0;
        }

        if (!cfg.getDouble(secName, "InitVelocity", init_data.init_velocity))
        {
            init_data.init_velocity = 0.0;
        }

        if (!cfg.getString(secName, "Profile", init_data.profile_path))
        {
            init_data.profile_path = "default";
        }

        if (!cfg.getDouble(secName, "ProfileStep", init_data.prof_step))
        {
            init_data.prof_step = 100.0;
        }

        if (!cfg.getString(secName, "TrainConfig", init_data.train_config))
        {
            init_data.train_config = "default-train";
        }

        if (!cfg.getInt(secName, "IntegrationTimeInterval", init_data.integration_time_interval))
        {
            init_data.integration_time_interval = 100;
        }

        if (!cfg.getInt(secName, "ControlTimeInterval", init_data.control_time_interval))
        {
            init_data.control_time_interval = 50;
        }

        control_delay = static_cast<double>(init_data.control_time_interval) / 1000.0;

        if (!cfg.getBool(secName, "DebugPrint", init_data.debug_print))
        {
            init_data.debug_print = false;
        }

        if (!cfg.getInt(secName, "KeysBufferSize", init_data.keys_buffer_size))
        {
            init_data.keys_buffer_size = 1024;
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
        init_data.train_config = command_line.train_config.value;

    if (command_line.route_dir.is_present)
        init_data.route_dir = command_line.route_dir.value;

    if (command_line.debug_print.is_present)
        init_data.debug_print = command_line.debug_print.value;

    if (command_line.init_coord.is_present)
    {
        init_data.init_coord = command_line.init_coord.value;        
    }

    if (command_line.direction.is_present)
        init_data.direction = command_line.direction.value;

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
            solver_config.method = "rkf5";
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
            solver_config.step = 1e-4;
        }

        Journal::instance()->info("Initial integration step: " + QString("%1").arg(solver_config.step));

        if (!cfg.getDouble(secName, "MaxStep", solver_config.max_step))
        {
            solver_config.max_step = 1e-2;
        }

        Journal::instance()->info("Maximal integration step: " + QString("%1").arg(solver_config.max_step));
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
    if (train->getTrainID().isEmpty())
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
    }
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
    if (sim_client == Q_NULLPTR)
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

    sim_client->sendTrainData(train_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::sharedMemoryFeedback()
{
    std::vector<Vehicle *> *vehicles = train->getVehicles();

    viewer_data.time = static_cast<float>(t);

    size_t i = 0;

    for (auto it = vehicles->begin(); it != vehicles->end(); ++it)
    {
        viewer_data.te[i].coord = static_cast<float>((*it)->getRailwayCoord());
        viewer_data.te[i].velocity = static_cast<float>((*it)->getVelocity());

        viewer_data.te[i].angle = static_cast<float>((*it)->getWheelAngle(0));
        viewer_data.te[i].omega = static_cast<float>((*it)->getWheelOmega(0));

        /*VehicleController *vc = topology.getVehicleController(i);

        vec3d att;
        vec3d pos = vc->getPosition(att);

        QString topDbg = QString("Тр: %1 Тр. коорд: %2 x: %3 y: %4 z: %5")
                .arg(vc->getCurrentTraj()->getName(), 10)
                .arg(vc->getTrajCoord(), 7, 'f', 2)
                .arg(pos.x(), 8, 'f', 1)
                .arg(pos.y(), 8, 'f', 1)
                .arg(pos.z(), 8, 'f', 1);*/

        //topDbg.toWCharArray(viewer_data.te[i].DebugMsg);
        (*it)->getDebugMsg().toWCharArray(viewer_data.te[i].DebugMsg);

        /*std::copy((*it)->getDiscreteSignals().begin(),
                  (*it)->getDiscreteSignals().end(),
                  viewer_data.te[i].discreteSignal.begin());*/

        std::copy((*it)->getAnalogSignals().begin(),
                  (*it)->getAnalogSignals().end(),
                  viewer_data.te[i].analogSignal.begin());

        ++i;
    }

    if (shared_memory.lock())
    {
        memcpy(shared_memory.data(), &viewer_data, sizeof (server_data_t));
        shared_memory.unlock();
    }

    viewer_data.count++;    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::controlStep(double &control_time, const double control_delay)
{
    if (control_time >= control_delay)
    {
        control_time = 0;

        if (keys_data.lock())
        {            
            data.resize(keys_data.size());
            memcpy(data.data(), keys_data.data(), static_cast<size_t>(keys_data.size()));

            if (keys_data.size() != 0)
                emit sendDataToTrain(data);

            keys_data.unlock();
        }
    }

    control_time += dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::topologyStep()
{
    for (size_t i = 0; i < train->getVehicles()->size(); ++i)
    {
        VehicleController *vc = topology.getVehicleController(i);
        vc[i].setRailwayCoord(train->getVehicles()->at(i)->getRailwayCoord());
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::process()
{
    double tau = 0;
    double integration_time = static_cast<double>(integration_time_interval) / 1000.0;    

    // Integrate all ODE in train motion model
    while ( (tau <= integration_time) &&
            is_step_correct)
    {
        preStep(t);

        // Feedback to viewer
        sharedMemoryFeedback();

        controlStep(control_time, control_delay);        

        is_step_correct = step(t, dt);

        //topologyStep();

        tau += dt;
        t += dt;

        postStep(t);
    }

    train->inputProcess();    

    // Debug print, is allowed
    if (is_debug_print)
        debugPrint();    
}

