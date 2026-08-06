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

#include    "Config.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Config::load(const QString& configPath)
{
    CfgReader cfg;

    if (!cfg.load(configPath))
    {
        qWarning() << "Failed to load config file:" << configPath;
        return false;
    }

    // Server section
    QDomNode serverNode = cfg.getFirstSection("Server");
    if (!serverNode.isNull())
    {
        int port;
        if (cfg.getInt(serverNode, "Port", port))
        {
            m_serverPort = static_cast<quint16>(port);
        }
        else
        {
            m_serverPort = 12345;
        }

        if (!cfg.getInt(serverNode, "MaxClients", m_maxClients))
        {
            m_maxClients = 10;
        }

        if (!cfg.getInt(serverNode, "TimeoutSeconds", m_timeoutSeconds))
        {
            m_timeoutSeconds = 30;
        }

        if (!cfg.getBool(serverNode, "AutoRestart", m_autoRestart))
        {
            m_autoRestart = true;
        }

        if (!cfg.getInt(serverNode, "RestartDelaySeconds", m_restartDelaySeconds))
        {
            m_restartDelaySeconds = 5;
        }
    }

    // Paths section
    QDomNode pathsNode = cfg.getFirstSection("Paths");
    if (!pathsNode.isNull())
    {
        if (!cfg.getString(pathsNode, "SimulatorPath", m_simulatorPath))
        {
            m_simulatorPath = "./bin/simulator";
        }

        if (!cfg.getString(pathsNode, "RoutesPath", m_routesPath))
        {
            m_routesPath = "./routes";
        }

        if (!cfg.getString(pathsNode, "LogFile", m_logFile))
        {
            m_logFile = "/var/log/simulator-server/server.log";
        }

        if (!cfg.getString(pathsNode, "PidFile", m_pidFile))
        {
            m_pidFile = "/var/run/simulator-server.pid";
        }
    }

    // Logging section
    QDomNode loggingNode = cfg.getFirstSection("Logging");
    if (!loggingNode.isNull())
    {
        if (!cfg.getString(loggingNode, "Level", m_logLevel))
        {
            m_logLevel = "info";
        }

        if (!cfg.getInt(loggingNode, "MaxFileSize", m_maxLogFileSize))
        {
            m_maxLogFileSize = 10485760; // 10 MB
        }

        if (!cfg.getInt(loggingNode, "MaxBackupFiles", m_maxBackupFiles))
        {
            m_maxBackupFiles = 5;
        }
    }

    // Security section
    QDomNode securityNode = cfg.getFirstSection("Security");
    if (!securityNode.isNull())
    {
        if (!cfg.getString(securityNode, "User", m_user))
        {
            m_user = "simulator";
        }

        if (!cfg.getString(securityNode, "Group", m_group))
        {
            m_group = "simulator";
        }

        if (!cfg.getInt(securityNode, "Umask", m_umask))
        {
            m_umask = 027;
        }
    }

    qInfo() << "Configuration loaded from:" << configPath;
    qInfo() << "  Port:" << m_serverPort;
    qInfo() << "  Routes path:" << m_routesPath;
    qInfo() << "  Simulator path:" << m_simulatorPath;

    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Config::save(const QString& configPath)
{
    // Сохранение конфигурации в XML (опционально)
    // Можно реализовать при необходимости
    Q_UNUSED(configPath)
    return true;
}