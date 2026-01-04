#ifndef     SCENARIO_MANAGER_H
#define     SCENARIO_MANAGER_H

#include    <QObject>
#include    <sol/sol.hpp>

#include    <scenario-manager-export.h>
#include    <scenario-train-data.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SCNMGR_EXPORT ScenarioManager : public QObject
{
    Q_OBJECT

public:

    ScenarioManager(QObject *parent = nullptr);

    ~ScenarioManager();

    void init();

    bool run(const std::string &script_path);

    std::vector<scenario_train_data_t> trains_data;

private:

    /// Контекст интерпретатора Lua
    sol::state lua;

    /// Регистрация всех доступных типов данных, их параметров и методов
    void types_registration();

    /// Создать поезд игрока
    void setTrain(const scenario_train_data_t &train_data);
};

#endif
