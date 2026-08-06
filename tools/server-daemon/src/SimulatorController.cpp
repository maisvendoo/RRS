//------------------------------------------------------------------------------
//
//  Simulator controller
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Simulator controller
 *  \copyright SimulatorServer
 *  \date 2026
 */

#include    "SimulatorController.h"
#include    <QDebug>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SimulatorController::SimulatorController(QObject* parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_isStarting(false)
    , m_startTime(0)
{
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &SimulatorController::checkProcessTimeout);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
SimulatorController::~SimulatorController()
{
    stopSimulation();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool SimulatorController::startSimulation(const QString& route,
                                          const QString& scenario,
                                          const QString& simulatorPath)
{
    if (isRunning())
    {
        emit simulationError("Simulation is already running");
        return false;
    }

    m_simulatorPath = simulatorPath;
    QString command = QString("%1 --route=%2 --scenario=%3")
                      .arg(simulatorPath)
                      .arg(route)
                      .arg(scenario);

    qInfo() << "Starting simulation:" << command;

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->setProgram(simulatorPath);
    m_process->setArguments(QStringList() << "--route=" + route << "--scenario=" + scenario);

    connect(m_process, &QProcess::finished,
            this, &SimulatorController::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &SimulatorController::onProcessError);
    connect(m_process, &QProcess::readyRead,
            this, &SimulatorController::onProcessOutput);

    m_currentRoute = route;
    m_currentScenario = scenario;
    m_isStarting = true;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_timeoutTimer->start(5000);

    m_process->start();
    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool SimulatorController::stopSimulation()
{
    if (!m_process)
    {
        return false;
    }

    if (m_process->state() == QProcess::Running)
    {
        m_process->terminate();
        if (!m_process->waitForFinished(3000))
        {
            m_process->kill();
        }
    }

    emit simulationStopped();
    m_isStarting = false;
    m_startTime = 0;
    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
qint64 SimulatorController::getUptimeSeconds() const
{
    if (!isRunning() || m_startTime == 0)
    {
        return 0;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    return (now - m_startTime) / 1000;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SimulatorController::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_isStarting = false;
    m_timeoutTimer->stop();

    emit processExit(exitCode);

    if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
        emit simulationStopped();
    }
    else
    {
        emit simulationError(QString("Simulation exited with code: %1").arg(exitCode));
    }

    m_process->deleteLater();
    m_process = nullptr;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SimulatorController::onProcessError(QProcess::ProcessError error)
{
    m_isStarting = false;
    m_timeoutTimer->stop();

    QString errorMsg;
    switch (error)
    {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start simulator";
            break;
        case QProcess::Crashed:
            errorMsg = "Simulator crashed";
            break;
        case QProcess::Timedout:
            errorMsg = "Simulator timed out";
            break;
        default:
            errorMsg = "Unknown error";
    }

    emit simulationError(errorMsg);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SimulatorController::onProcessOutput()
{
    if (m_process)
    {
        QString output = m_process->readAll();
        if (!output.isEmpty())
        {
            emit simulationOutput(output);
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void SimulatorController::checkProcessTimeout()
{
    if (m_isStarting)
    {
        m_isStarting = false;
        if (m_process && m_process->state() != QProcess::Running)
        {
            emit simulationError("Simulation start timed out");
            m_process->deleteLater();
            m_process = nullptr;
        }
    }
}