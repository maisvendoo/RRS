//------------------------------------------------------------------------------
//
//  Server configuration
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Server configuration
 *  \copyright SimulatorServer
 *  \date 2026
 */

#ifndef     CONFIG_H
#define     CONFIG_H

#include    <QString>
#include    <QDebug>
#include    "CfgReader.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class Config
{
public:

    static Config& instance()
    {
        static Config config;
        return config;
    }

    bool load(const QString& configPath = "config/server_config.xml");
    bool save(const QString& configPath = "config/server_config.xml");

    // Геттеры
    quint16 getServerPort() const { return m_serverPort; }
    int getMaxClients() const { return m_maxClients; }
    int getTimeoutSeconds() const { return m_timeoutSeconds; }
    bool getAutoRestart() const { return m_autoRestart; }
    int getRestartDelaySeconds() const { return m_restartDelaySeconds; }

    QString getSimulatorPath() const { return m_simulatorPath; }
    QString getRoutesPath() const { return m_routesPath; }
    QString getLogFile() const { return m_logFile; }
    QString getPidFile() const { return m_pidFile; }

    QString getLogLevel() const { return m_logLevel; }
    int getMaxLogFileSize() const { return m_maxLogFileSize; }
    int getMaxBackupFiles() const { return m_maxBackupFiles; }

    QString getUser() const { return m_user; }
    QString getGroup() const { return m_group; }
    int getUmask() const { return m_umask; }

    // Сеттеры
    void setServerPort(quint16 port) { m_serverPort = port; }
    void setMaxClients(int clients) { m_maxClients = clients; }
    void setSimulatorPath(const QString& path) { m_simulatorPath = path; }
    void setRoutesPath(const QString& path) { m_routesPath = path; }
    void setLogFile(const QString& file) { m_logFile = file; }

private:

    Config() = default;

    // Server settings
    quint16     m_serverPort;
    int         m_maxClients;
    int         m_timeoutSeconds;
    bool        m_autoRestart;
    int         m_restartDelaySeconds;

    // Paths
    QString     m_simulatorPath;
    QString     m_routesPath;
    QString     m_logFile;
    QString     m_pidFile;

    // Logging
    QString     m_logLevel;
    int         m_maxLogFileSize;
    int         m_maxBackupFiles;

    // Security
    QString     m_user;
    QString     m_group;
    int         m_umask;
};

#endif // CONFIG_H