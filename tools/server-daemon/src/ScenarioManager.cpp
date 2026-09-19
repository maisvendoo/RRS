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

    // Получаем список подкаталогов в папке scenarios
    QStringList scenarioDirs = scenariosDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (scenarioDirs.isEmpty())
    {
        qWarning() << "No scenario directories found in:" << scenariosPath;
        return false;
    }

    QString routeName = QFileInfo(routePath).fileName();

    for (const QString& dirName : scenarioDirs)
    {
        QString scenarioPath = scenariosDir.absolutePath() + "/" + dirName;
        
        ScenarioInfo info;
        info.dirName = dirName;
        info.name = dirName;  // По умолчанию имя = имя каталога
        info.path = scenarioPath;
        info.route = routeName;
        info.description = "";

        // Загружаем информацию о сценарии
        loadScenarioInfo(scenarioPath, info, routeName);

        m_scenarios.append(info);
        qInfo() << "Loaded scenario:" << info.name;
    }

    return !m_scenarios.isEmpty();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ScenarioManager::loadScenarioInfo(const QString& scenarioPath, ScenarioInfo& info, const QString& routeName)
{
    // Пытаемся найти файл описания сценария
    QString descFile = scenarioPath + "/description.xml";
    if (!QFile::exists(descFile))
    {
        descFile = scenarioPath + "/scenario.xml";
    }
    
    if (QFile::exists(descFile))
    {
        return parseScenarioDescription(descFile, info);
    }
    
    // Если файла описания нет, используем имя папки как имя сценария
    info.name = info.dirName;
    info.description = "";
    info.route = routeName;
    
    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ScenarioManager::parseScenarioDescription(const QString& filePath, ScenarioInfo& info)
{
    CfgReader cfg;

    if (!cfg.load(filePath))
    {
        qWarning() << "Failed to load scenario description:" << filePath;
        return false;
    }

    // Пробуем получить секцию Scenario
    QDomNode rootNode = cfg.getFirstSection("Scenario");
    if (rootNode.isNull())
    {
        // Пробуем Config
        rootNode = cfg.getFirstSection("Config");
        if (rootNode.isNull())
        {
            qWarning() << "Unknown scenario format in:" << filePath;
            return false;
        }
    }

    // Получаем имя сценария (если указано)
    QString name;
    if (cfg.getString(rootNode, "Name", name))
    {
        if (!name.isEmpty())
        {
            info.name = name;
        }
    }

    // Получаем описание
    QString description;
    if (cfg.getString(rootNode, "Description", description))
    {
        info.description = description;
    }

    return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ScenarioInfo ScenarioManager::getScenarioByName(const QString& name) const
{
    for (const ScenarioInfo& scenario : m_scenarios)
    {
        if (scenario.name == name || scenario.dirName == name)
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
    if (info.dirName.isEmpty())
    {
        return QString();
    }
    return m_currentRoutePath + "/scenarios/" + info.dirName;
}