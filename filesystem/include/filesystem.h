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

    QString getWorkingDirectory() const;

    QString getHomeDirectory() const;

    QString getSimDataDirectory() const;

    QString getLogsDirectory() const;

private:

    /// Working directory
    QString workingDir;
    /// User home directory
    QString homeDir;
    /// Simulator data directory
    QString simDataDir;
    /// Logs directory
    QString logsDirectory;

    /// Directory creation
    void createDirectory(const QString &path);

    /// Get directory path with separator
    QString getDirectoryPath(QString path) const;
};

#endif // FILESYSTEM_H
