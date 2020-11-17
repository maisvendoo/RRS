#ifndef SIMRUN_H
#define SIMRUN_H
#include <QObject>
#include <QProcess>
#include "SimParameters.h"

#ifdef QT_DEBUG
    const QString SIMULATOR_NAME = "simulator_d";
    const QString VIEWER_NAME = "viewer_d";
#else
    const QString SIMULATOR_NAME = "simulator";
    const QString VIEWER_NAME = "viewer";
#endif

#ifdef __WIN32__
    const QString EXE_EXP = ".exe";
#else
    const QString EXE_EXP = "";
#endif

class SimRun : public QObject
{
    Q_OBJECT
public:
    SimRun(SimParameters* param, QObject* parent = nullptr);
    Q_INVOKABLE void startSimulator();

private:
    void startViewer();
    SimParameters* m_param;
    QProcess       m_simulatorProc;
    QProcess       m_viewerProc;
};

#endif // SIMRUN_H
