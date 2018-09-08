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

const       QString LOG_FILE_NAME = "simulator.log";

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Model::Model(QObject *parent) : QObject(parent)
  , simLog(Q_NULLPTR)
  , t(0.0)
  , dt(0.001)
  , start_time(0.0)
  , stop_time(1000.0)
  , is_step_correct(true)
  , is_simulation_started(false)
  , realtime_delay(0)
  , integration_time_interval(100)
  , is_debug_print(false)
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
bool Model::init(const command_line_t &command_line)
{
    // Log creation
    logInit(command_line.clear_log.is_present);

    // Check is debug print allowed
    is_debug_print = command_line.debug_print.is_present;

    init_data_t init_data;

    // Load initial data configuration
    loadInitData(init_data);

    // Override init data by command line
    overrideByCommandLine(init_data, command_line);

    // Read solver configuration
    configSolver(init_data.solver_config);

    start_time = init_data.solver_config.start_time;
    stop_time = init_data.solver_config.stop_time;
    dt = init_data.solver_config.step;

    // Train creation and initialization
    train = new Train(&fs);

    connect(train, &Train::logMessage, this, &Model::logMessage);

    if (!train->init(init_data))
        return false;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::start()
{
    if (!isStarted())
    {
        this->moveToThread(&model_thread);

        connect(&model_thread, &QThread::started,
                this, &Model::process);

        is_simulation_started = true;

        model_thread.start();
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
void Model::process()
{
    // Current time initialization
    t = start_time;

    // Main simulation loop
    while ( (t <= stop_time) &&
            is_step_correct &&
            is_simulation_started)
    {
        QTime solveTime;

        int solve_time = 0;
        solveTime.start();

        double tau = 0;
        double integration_time = static_cast<double>(integration_time_interval) / 1000.0;

        // Integrate all ODE in train motion model
        while ( (tau <= integration_time) &&
                is_step_correct)
        {
            preStep(t);

            is_step_correct = step(t, dt);

            tau += dt;
            t += dt;

            postStep(t);
        }

        // Debug print, is allowed
        if (is_debug_print)
            debugPrint();

        solve_time = solveTime.elapsed();

        // Make delay for realtime simulation
        realtime_delay = integration_time_interval - solve_time;

        if (realtime_delay > 0)
            QThread::msleep(realtime_delay);
    }
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
void Model::logInit(bool clear_log)
{
    simLog = new Log(fs.getLogsDirectory() + LOG_FILE_NAME, clear_log, true);
    connect(this, &Model::logMessage, simLog, &Log::printMessage);
    connect(this, &Model::logMessage, this, &Model::outMessage);
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
    QString debug_info = QString("t = %1 realtime_delay = %2 time_step = %3 v[first] = %4 v[last] = %5\n")
            .arg(t)
            .arg(realtime_delay)
            .arg(dt)
            .arg(train->getVelocity(0) * 3.6)
            .arg(train->getVelocity(99) * 3.6);

    fputs(qPrintable(debug_info), stdout);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::loadInitData(init_data_t &init_data)
{
    CfgReader cfg;
    QString cfg_path = fs.getConfigDirectory() + "init-data.xml";

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

        if (!cfg.getString(secName, "TrainConfig", init_data.train_config_path))
        {
            init_data.train_config_path = "default-train";
        }

        if (!cfg.getInt(secName, "IntegrationTimeInterval", init_data.integration_time_interval))
        {
            init_data.integration_time_interval = 100;
        }

        if (!cfg.getBool(secName, "DebugPrint", init_data.debug_print))
        {
            init_data.debug_print = false;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::overrideByCommandLine(init_data_t &init_data,
                                  const command_line_t &command_line)
{
    if (command_line.train_config.is_present)
        init_data.train_config_path = command_line.train_config.value;

    if (command_line.debug_print.is_present)
        init_data.debug_print = command_line.debug_print.value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::configSolver(solver_config_t &solver_config)
{
    CfgReader cfg;
    QString cfg_path = fs.getConfigDirectory() + "solver.xml";

    if (cfg.load(cfg_path))
    {
        QString secName = "Solver";

        if (!cfg.getString(secName, "Method", solver_config.method))
        {
            solver_config.method = "rkf5";
        }

        if (!cfg.getDouble(secName, "StartTime", solver_config.start_time))
        {
            solver_config.start_time = 0;
        }

        if (!cfg.getDouble(secName, "StopTime", solver_config.stop_time))
        {
            solver_config.stop_time = 10.0;
        }

        if (!cfg.getDouble(secName, "InitStep", solver_config.step))
        {
            solver_config.step = 1e-4;
        }
    }
}


