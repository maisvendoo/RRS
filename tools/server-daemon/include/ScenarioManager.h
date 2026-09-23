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

#ifndef     SCENARIOMANAGER_H
#define     SCENARIOMANAGER_H

#include    <QObject>
#include    <QString>
#include    <QVector>
#include    <QDir>
#include    <QFileInfo>
#include    "CfgReader.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
struct ScenarioInfo
{
    QString     name;           // Имя сценария (из description.xml или имя папки)
    QString     dirName;        // Имя каталога сценария
    QString     description;    // Описание сценария
    QString     route;          // Маршрут, к которому относится сценарий
    QString     path;           // Полный путь к каталогу сценария
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class ScenarioManager : public QObject
{
    Q_OBJECT

public:

    explicit ScenarioManager(QObject* parent = nullptr);

    bool loadScenarios(const QString& routePath);
    QVector<ScenarioInfo> getScenarios() const { return m_scenarios; }
    ScenarioInfo getScenarioByName(const QString& name) const;
    QString getScenarioPath(const QString& scenarioName) const;
    bool hasScenarios() const { return !m_scenarios.isEmpty(); }
    int getScenariosCount() const { return m_scenarios.size(); }
    void clear() { m_scenarios.clear(); }

private:

    bool parseScenarioDescription(const QString& filePath, ScenarioInfo& info);
    bool loadScenarioInfo(const QString& scenarioPath, ScenarioInfo& info, const QString& routeName);

    QVector<ScenarioInfo>   m_scenarios;
    QString                 m_currentRoutePath;
};

#endif // SCENARIOMANAGER_H