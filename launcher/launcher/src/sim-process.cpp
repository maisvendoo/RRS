//------------------------------------------------------------------------------
//
//      Module for work with external process
//      (c) maisvendoo, 25/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Module for work with external process
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 25/09/2018
 */

#include    "sim-process.h"

#ifdef QT_DEBUG
    const QString PROGRAM_NAME = "simulator_d";
#else
    const QString PROGRAM_NAME = "simulator";
#endif

#if defined(Q_OS_WIN)
    const QString SimProcess::SIMULATOR_EXECUTABLE = PROGRAM_NAME + ".exe";
#else
    const QString SimProcess::SIMULATOR_EXECUTABLE = PROGRAM_NAME;
#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SimProcess::SimProcess(FileSystem *fs, QObject *parent) : QObject(parent)
  , fs(fs)
  , proc(new QProcess(this))
  , train_config_name("default")
  , clear_log(false)
  , debug_print(true)
{
    connect(proc, &QProcess::readyReadStandardOutput,
            this, &SimProcess::readStdOutput);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SimProcess::~SimProcess()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimProcess::start()
{
    QString program = SIMULATOR_EXECUTABLE;

    QStringList args;

    args << "--train-config=" + train_config_name;

    if (clear_log)
        args << "--clear-log";

    if (debug_print)
        args << "--debug-print";

    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->setWorkingDirectory(fs->getWorkingDirectory());
    proc->start(program, args);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimProcess::abort()
{
    proc->close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimProcess::setTrainConfig(QString train_config_name)
{
    this->train_config_name = train_config_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimProcess::setDebugPrint(bool debug_print)
{
    this->debug_print = debug_print;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimProcess::setClearLog(bool clear_log)
{
    this->clear_log = clear_log;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SimProcess::readStdOutput()
{
    emit printOutput(proc->readAllStandardOutput());
}
