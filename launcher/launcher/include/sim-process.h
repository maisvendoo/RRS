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

#ifndef     PROCESS_H
#define     PROCESS_H

#include    <QObject>
#include    <QProcess>

#include    "filesystem.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SimProcess : public QObject
{
    Q_OBJECT

public:

    explicit SimProcess(FileSystem *fs, QObject *parent = Q_NULLPTR);
    virtual ~SimProcess();

signals:

    /// Print process output to external viewer
    void printOutput(QString text);

public slots:

    /// Start simulation process
    void start();
    /// Stop simulation process
    void abort();

private:

    static const QString SIMULATOR_EXECUTABLE;

    /// Pointer to filesystem object of launcher
    FileSystem  *fs;
    /// Simulation process object
    QProcess    *proc;

    QString     train_config_name;
    bool        clear_log;
    bool        debug_output;

private slots:

    void readStdOutput();
};

#endif // PROCESS_H
