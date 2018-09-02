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
void Model::logInit(bool clear_log)
{
    simLog = new Log(fs.getLogsDirectory() + LOG_FILE_NAME, clear_log, true);
    connect(this, &Model::logMessage, simLog, &Log::printMessage);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::preStep(double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Model::step(double t, double &dt)
{
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::postStep(double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Model::debugPrint()
{
    QString debug_info = QString("t = %1 realtime_delay = %2\n")
            .arg(t)
            .arg(realtime_delay);

    fputs(qPrintable(debug_info), stdout);
}


