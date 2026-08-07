//------------------------------------------------------------------------------
//
//  Simulator controller
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------

#include    "SimulatorController.h"
#include    <QDebug>
#include    <QDir>

//-----------------------------------------------------------------------------
SimulatorController::SimulatorController(QObject* parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_isStarting(false)
    , m_startTime(0)
    , m_isStopping(false)
    , m_processState(ProcessState::Stopped)
{
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &SimulatorController::checkProcessTimeout);
}

//-----------------------------------------------------------------------------
SimulatorController::~SimulatorController()
{
    stopSimulation();
}

//-----------------------------------------------------------------------------
bool SimulatorController::startSimulation(const QString& route,
                                          const QString& scenario,
                                          const QString& simulatorPath)
{
    if (m_processState == ProcessState::Running || m_processState == ProcessState::Starting)
    {
        emit simulationError("Simulation is already running");
        return false;
    }

    // Очищаем предыдущий процесс если он есть
    if (m_process)
    {
        m_process->disconnect();
        m_process->deleteLater();
        m_process = nullptr;
    }

    if (!QFile::exists(simulatorPath))
    {
        emit simulationError("Simulator executable not found: " + simulatorPath);
        return false;
    }

    QStringList arguments;
    arguments << "--route=" + route;
    arguments << "--scenario=" + scenario;

    qInfo() << "Starting simulation:" << simulatorPath << arguments.join(" ");

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->setProgram(simulatorPath);
    m_process->setArguments(arguments);

    connect(m_process, &QProcess::finished,
            this, &SimulatorController::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &SimulatorController::onProcessError);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &SimulatorController::onProcessOutput);

    m_currentRoute = route;
    m_currentScenario = scenario;
    m_isStarting = true;
    m_isStopping = false;
    m_processState = ProcessState::Starting;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_timeoutTimer->start(5000);

    m_process->start();
    
    if (!m_process->waitForStarted(3000))
    {
        emit simulationError("Failed to start simulator process");
        m_process->disconnect();
        m_process->deleteLater();
        m_process = nullptr;
        m_processState = ProcessState::Stopped;
        return false;
    }
    
    m_processState = ProcessState::Running;
    emit simulationStarted();
    return true;
}

//-----------------------------------------------------------------------------
bool SimulatorController::stopSimulation()
{
    // Защита от повторного вызова
    if (m_isStopping)
    {
        qInfo() << "Stop already in progress";
        return true;
    }

    if (!m_process)
    {
        m_processState = ProcessState::Stopped;
        qInfo() << "No process to stop";
        return true;
    }

    if (m_process->state() != QProcess::Running && m_process->state() != QProcess::Starting)
    {
        qInfo() << "Process not running, state:" << m_process->state();
        m_process->disconnect();
        m_process->deleteLater();
        m_process = nullptr;
        m_processState = ProcessState::Stopped;
        return true;
    }

    m_isStopping = true;
    qInfo() << "Stopping simulation (PID:" << m_process->processId() << ")...";

    // Отключаем сигналы, чтобы избежать race conditions
    m_process->disconnect();

    // Пытаемся завершить процесс
    m_process->terminate();

    // Ждем завершения с таймаутом
    bool finished = m_process->waitForFinished(5000);

    if (!finished)
    {
        qWarning() << "Process didn't terminate, killing...";
        m_process->kill();
        m_process->waitForFinished(2000);
    }

    // Очищаем
    m_process->deleteLater();
    m_process = nullptr;
    m_isStopping = false;
    m_isStarting = false;
    m_startTime = 0;
    m_processState = ProcessState::Stopped;

    emit simulationStopped();
    qInfo() << "Simulation stopped successfully";

    return true;
}

//-----------------------------------------------------------------------------
qint64 SimulatorController::getUptimeSeconds() const
{
    if (m_processState != ProcessState::Running || m_startTime == 0)
    {
        return 0;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    return (now - m_startTime) / 1000;
}

//-----------------------------------------------------------------------------
bool SimulatorController::isRunning() const
{
    return m_processState == ProcessState::Running && m_process && m_process->state() == QProcess::Running;
}

//-----------------------------------------------------------------------------
void SimulatorController::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Защита от двойного вызова
    if (m_isStopping)
    {
        qInfo() << "Process already handled by stop";
        return;
    }

    if (!m_process)
    {
        qInfo() << "Process already deleted";
        m_processState = ProcessState::Stopped;
        return;
    }

    m_isStarting = false;
    m_timeoutTimer->stop();

    qInfo() << "Process finished with code:" << exitCode << "status:" << exitStatus;

    if (exitStatus == QProcess::NormalExit)
    {
        if (exitCode == 0)
        {
            m_processState = ProcessState::Stopped;
            emit simulationStopped();
        }
        else
        {
            qWarning() << "Simulator exited with code:" << exitCode;
            m_processState = ProcessState::Stopped;
            emit simulationStopped();
        }
    }
    else
    {
        qWarning() << "Simulator crashed!";
        m_processState = ProcessState::Crashed;
        emit simulationError("Simulator crashed");
        emit simulationStopped();
    }

    // Безопасно удаляем процесс
    if (m_process)
    {
        m_process->disconnect();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

//-----------------------------------------------------------------------------
void SimulatorController::onProcessError(QProcess::ProcessError error)
{
    // Защита от двойного вызова
    if (m_isStopping)
    {
        qInfo() << "Ignoring error during stop";
        return;
    }

    if (!m_process)
    {
        qInfo() << "Process already deleted";
        m_processState = ProcessState::Stopped;
        return;
    }

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
            break;
    }

    if (!errorMsg.isEmpty())
    {
        emit simulationError(errorMsg);
    }

    m_processState = ProcessState::Stopped;
    emit simulationStopped();

    if (m_process)
    {
        m_process->disconnect();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

//-----------------------------------------------------------------------------
void SimulatorController::onProcessOutput()
{
    if (m_process && !m_isStopping && m_processState == ProcessState::Running)
    {
        QString output = m_process->readAllStandardOutput();
        if (!output.isEmpty())
        {
            emit simulationOutput(output);
        }
    }
}

//-----------------------------------------------------------------------------
void SimulatorController::checkProcessTimeout()
{
    if (m_isStarting && m_process)
    {
        m_isStarting = false;
        if (m_process->state() != QProcess::Running)
        {
            m_processState = ProcessState::Stopped;
            emit simulationError("Simulation start timed out");
            m_process->disconnect();
            m_process->deleteLater();
            m_process = nullptr;
        }
    }
}
