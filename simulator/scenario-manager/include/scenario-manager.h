#ifndef     SCENARIO_MANAGER_H
#define     SCENARIO_MANAGER_H

#include    <QObject>
#include    <scenario-manager-export.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SCNMGR_EXPORT ScenarioManager : public QObject
{
    Q_OBJECT

public:

    ScenarioManager(QObject *parent = nullptr);

    ~ScenarioManager();

private:


};

#endif
