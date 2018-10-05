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
const       QString CFG_DIR = "cfg";
const       QString TRAINS_DIR = "trains";

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FileSystem::FileSystem()
{
    // Current working directory
    workingDir = QDir::currentPath();

    // Calculate simulator root directory
    QDir workDir(workingDir);
    workDir.cdUp();
    rootDirectory = workDir.path();

    // Libraries directories
    libDirectory = combinePath(rootDirectory, "lib");
    modulesDirectory = combinePath(rootDirectory, "modules");

    // Config directories
    configDirectory = combinePath(rootDirectory, "cfg");
    trainsDirectory = combinePath(configDirectory, "trains");
    vehiclesDirectory = combinePath(configDirectory, "vehicles");
    couplingsDirectory = combinePath(configDirectory, "couplings");

    // Routes directory
    routesDirectory = combinePath(rootDirectory, "routes");

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
QString FileSystem::getRootDirectory() const
{
    return getDirectoryPath(rootDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getLibDirectory() const
{
    return getDirectoryPath(libDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getModulesDirectory() const
{
    return getDirectoryPath(modulesDirectory);
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
QString FileSystem::getConfigDirectory() const
{
    return getDirectoryPath(configDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getTrainsDirectory() const
{
    return getDirectoryPath(trainsDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getVehiclesDirectory() const
{
    return getDirectoryPath(vehiclesDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getCouplingsDirectory() const
{
    return getDirectoryPath(couplingsDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::getRoutesDirectory() const
{
    return getDirectoryPath(routesDirectory);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString FileSystem::combinePath(const QString &path1, const QString &path2)
{
    if (*(path1.end() - 1) != QDir::separator())
        return QDir::toNativeSeparators(path1 + QDir::separator() + path2);
    else
        return QDir::toNativeSeparators(path1 + path2);
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
