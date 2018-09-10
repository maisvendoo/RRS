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

#ifndef     FILESYSTEM_H
#define     FILESYSTEM_H

#include    <QtGlobal>
#include    <QString>

#if defined(FILESYSTEM_LIB)
    #define FILESYSTEM_EXPORT Q_DECL_EXPORT
#else
    #define FILESYSTEM_EXPORT Q_DECL_IMPORT
#endif

/*!
 * \class
 * \brief Filesystem control
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FILESYSTEM_EXPORT FileSystem
{
public:

    /// Constructor
    FileSystem();
    /// Destructor
    virtual ~FileSystem();

    /// Get absolute path to simulator root directory
    QString getRootDirectory() const;
    /// Get libraries path
    QString getLibDirectory() const;
    /// Get modules directory
    QString getModulesDirectory() const;
    /// Get absolute path to working directory
    QString getWorkingDirectory() const;
    /// Get absolute path to user home directory
    QString getHomeDirectory() const;
    /// Get absolute path to simulator output data directory
    QString getSimDataDirectory() const;
    /// Get absolute path to log files directory
    QString getLogsDirectory() const;
    /// Get config directory
    QString getConfigDirectory() const;
    /// Get trains config directory
    QString getTrainsDirectory() const;
    /// Get vehicles config directory
    QString getVehiclesDirectory() const;
    /// Get couplings directory
    QString getCouplingsDirectory() const;
    /// Get routes directory
    QString getRoutesDirectory() const;

    QString combinePath(const QString &path1, const QString &path2);

private:

    /// Root directory
    QString rootDirectory;
    /// Libraries path
    QString libDirectory;
    /// Modules directory
    QString modulesDirectory;
    /// Working directory
    QString workingDir;
    /// User home directory
    QString homeDir;
    /// Simulator data directory
    QString simDataDir;
    /// Logs directory
    QString logsDirectory;
    /// Config directory
    QString configDirectory;
    /// Trains config directory
    QString trainsDirectory;
    /// Vehicles config directory
    QString vehiclesDirectory;
    /// Couplings config directory
    QString couplingsDirectory;
    /// Routes directory
    QString routesDirectory;

    /// Directory creation
    void createDirectory(const QString &path);

    /// Get directory path with separator
    QString getDirectoryPath(QString path) const;
};

#endif // FILESYSTEM_H
