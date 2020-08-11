#include "SimRun.h"
#include "filesystem.h"

SimRun::SimRun(SimParameters *param, QObject *parent)
    : QObject(parent)
    , m_param(param)
{
    connect(&m_simulatorProc, &QProcess::started, this, &SimRun::startViewer);
    connect(&m_viewerProc, QOverload<int>::of(&QProcess::finished),this, [&](){
        m_simulatorProc.kill();
    });
}

#include <QDebug>
#include <QDir>
void SimRun::startSimulator()
{
    qDebug() << m_param->getOrdinate();
    if(m_simulatorProc.state() == QProcess::NotRunning
            && m_viewerProc.state() == QProcess::NotRunning)

    {
        const FileSystem &fs = FileSystem::getInstance();

        if(m_param->getTrain().isEmpty() || m_param->getRoute().isEmpty())
            return;

        QStringList args;
        args << "--train-config=" + m_param->getTrain();
        args << "--route=" + QDir::toNativeSeparators(m_param->getRoute());

        if(m_param->getDirection())
            args << "--direction=-1";
        else
            args << "--direction=1";

        if (!m_param->getWaypoint().isEmpty())
        {
            double init_coord = m_param->getOrdinate() / 1000.0;
            args << "--init-coord=" + QString("%1").arg(init_coord, 0, 'f', 2);
        }

        const QString simPath = SIMULATOR_NAME + EXE_EXP;

        m_simulatorProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
        m_simulatorProc.start(simPath, args);
    }
}

void SimRun::startViewer()
{
    QStringList args;
    args << "--route" <<  QDir::toNativeSeparators(m_param->getRoute());
    args << "--train" << m_param->getTrain();
    args << "--direction";

    if (m_param->getDirection())
        args << "-1";
    else
        args << "1";

    FileSystem &fs = FileSystem::getInstance();
    QString viewerPath = VIEWER_NAME + EXE_EXP;

    m_viewerProc.setWorkingDirectory(QString(fs.getBinaryDir().c_str()));
    m_viewerProc.start(viewerPath, args);
}
