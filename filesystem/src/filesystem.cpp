//------------------------------------------------------------------------------
//
//      Library for work with paths of files and directories
//      (c) maisvendoo, 02/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief  Library for work with paths of files and directories
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 02/09/2018
 */

#include    "filesystem.h"

#include    <QFileInfo>
#include    <QDir>

const       QString SIM_DATA_DIR = ".TrueRailway";
const       QString LOGS_DIR = "logs";

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FileSystem::FileSystem()
{
    workingDir = QDir::currentPath();
    homeDir = QDir::homePath();
    simDataDir = homeDir + QDir::separator() + SIM_DATA_DIR;

    createDirectory(simDataDir);

    logsDirectory = simDataDir + QDir::separator() + LOGS_DIR;

    createDirectory(logsDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FileSystem::~FileSystem()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getWorkingDirectory() const
{
    return getDirectoryPath(workingDir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getHomeDirectory() const
{
    return getDirectoryPath(homeDir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getSimDataDirectory() const
{
    return getDirectoryPath(simDataDir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getLogsDirectory() const
{
    return getDirectoryPath(logsDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FileSystem::createDirectory(const QString &path)
{
    QDir dir(path);

    if (!dir.exists())
        dir.mkdir(path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getDirectoryPath(QString path) const
{
    QString tmp =  path + QDir::separator();
    return tmp;
}
