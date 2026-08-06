//------------------------------------------------------------------------------
//
//  Scenario manager
//  (c) SimulatorServer 2026
//
//------------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Scenario manager
 *  \copyright SimulatorServer
 *  \date 2026
 */

#include    "ScenarioManager.h"
#include    <QDebug>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ScenarioManager::ScenarioManager(QObject* parent) : QObject(parent)
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ScenarioManager::loadScenarios(const QString& routePath)
{
    m_scenarios.clear();
    m_currentRoutePath = routePath;

    QString scenariosPath = routePath + "/scenarios";
    QDir scenariosDir(scenariosPath);

    if (!scenariosDir.exists())
    {
        qWarning() << "Scenarios directory does not exist:" << scenariosPath;
        return false;
    }

    QStringList filters;
    filters << "*.xml";
    QFileInfoList scenarioFiles = scenariosDir.entryInfoList(filters, QDir::Files);

    if (scenarioFiles.isEmpty())
    {
        qWarning() << "No scenario files found in:" << scenariosPath;
        return false;
    }

    QString routeName = QFileInfo(routePath).fileName();

    for (const QFileInfo& fileInfo : scenarioFiles)
    {
        ScenarioInfo info;
        info.fileName = fileInfo.fileName();
        info.filePath = fileInfo.absoluteFilePath();

        if (parseScenarioFile(fileInfo.absoluteFilePath(), info, routeName))
        {
            m_scenarios.append(info);
            qInfo() << "Loaded scenario:" << info.name << "from" << info.fileName;
        }
    }

    return !m_scenarios.isEmpty();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ScenarioManager::parseScenarioFile(const QString& filePath, ScenarioInfo& info, const QString& route)
{
    CfgReader cfg;

    if (!cfg.load(filePath))
    {
        qWarning() << "Failed to load scenario file:" << filePath;
        return false;
    }

    QDomNode rootNode = cfg.getFirstSection("Scenario");
    if (rootNode.isNull())
    {
        rootNode = cfg.getFirstSection("Config");
        if (rootNode.isNull())
        {
            qWarning() << "Unknown scenario format in:" << filePath;
            return false;
        }
    }

    QString name;
    if (cfg.getString(rootNode, "Name", name))
    {
        info.name = name;
    }
    else
    {
        info.name = QFileInfo(filePath).baseName();
    }

    QString description;
    if (cfg.getString(rootNode, "Description", description))
    {
        info.description = description;
    }

    QString type;
    if (cfg.getString(rootNode, "Type", type))
    {
        info.type = type;
    }

    info.route = route;
    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ScenarioInfo ScenarioManager::getScenarioByName(const QString& name) const
{
    for (const ScenarioInfo& scenario : m_scenarios)
    {
        if (scenario.name == name || scenario.fileName == name)
        {
            return scenario;
        }
    }
    return ScenarioInfo();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
QString ScenarioManager::getScenarioPath(const QString& scenarioName) const
{
    ScenarioInfo info = getScenarioByName(scenarioName);
    if (info.fileName.isEmpty())
    {
        return QString();
    }
    return m_currentRoutePath + "/scenarios/" + info.fileName;
}