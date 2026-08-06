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
    QString     name;           // Имя сценария
    QString     fileName;       // Имя файла сценария
    QString     description;    // Описание сценария
    QString     route;          // Маршрут, к которому относится сценарий
    QString     type;           // Тип сценария (если есть)
    QString     filePath;       // Полный путь к файлу сценария
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

    bool parseScenarioFile(const QString& filePath, ScenarioInfo& info, const QString& route);

    QVector<ScenarioInfo>   m_scenarios;
    QString                 m_currentRoutePath;
};

#endif // SCENARIOMANAGER_H