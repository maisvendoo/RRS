//------------------------------------------------------------------------------
//
//  Simulator controller
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------

#ifndef     SIMULATORCONTROLLER_H
#define     SIMULATORCONTROLLER_H

#include    <QObject>
#include    <QProcess>
#include    <QString>
#include    <QTimer>
#include    <QDateTime>
#include    <QFile>

//-----------------------------------------------------------------------------
class SimulatorController : public QObject
{
    Q_OBJECT

public:

    explicit SimulatorController(QObject* parent = nullptr);
    ~SimulatorController();

    bool startSimulation(const QString& route, const QString& scenario, const QString& simulatorPath);
    bool stopSimulation();
    bool isRunning() const;
    bool isStarting() const { return m_isStarting; }

    QString getCurrentRoute() const { return m_currentRoute; }
    QString getCurrentScenario() const { return m_currentScenario; }
    qint64 getStartTime() const { return m_startTime; }
    qint64 getUptimeSeconds() const;

signals:

    void simulationStarted();
    void simulationStopped();
    void simulationError(const QString& error);
    void simulationOutput(const QString& output);
    void processExit(int exitCode);

private slots:

    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessOutput();
    void checkProcessTimeout();

private:

    enum class ProcessState
    {
        Stopped,
        Starting,
        Running,
        Crashed
    };

    QProcess*   m_process;
    QString     m_currentRoute;
    QString     m_currentScenario;
    QTimer*     m_timeoutTimer;
    bool        m_isStarting;
    qint64      m_startTime;
    bool        m_isStopping;
    ProcessState m_processState;
};

#endif // SIMULATORCONTROLLER_H