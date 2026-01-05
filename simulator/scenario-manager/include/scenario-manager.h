#ifndef     SCENARIO_MANAGER_H
#define     SCENARIO_MANAGER_H

#include    <QObject>
#include    <sol/sol.hpp>

#include    <scenario-manager-export.h>
#include    <scenario-train-data.h>

#include    <init_data.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SCNMGR_EXPORT ScenarioManager : public QObject
{
    Q_OBJECT

public:

    ScenarioManager(QObject *parent = nullptr);

    ~ScenarioManager();

    /// Общая инициализация интерпретатора и окружения
    void init(const init_data_t &init_data);

    /// Запуск на исполнение скрипта сценария
    bool run(const std::string &script_path);    

    /// Данные инициализации симулятора
    std::vector<init_data_t> init_datas;

    std::int64_t getStartDateTime() const
    {
        return launch_init_data.start_datetime;
    }

private:

    /// Контекст интерпретатора Lua
    sol::state lua;

    /// Данные инициализации, полученные от лаунчера
    init_data_t launch_init_data;

    /// Регистрация всех доступных типов данных, их параметров и методов
    void types_registration();

    /// Создать поезд игрока
    void setTrain(const scenario_train_data_t &train_data);

    /// Установить время сервера (формат строки "hh:mm:ss"
    void setTime(const std::string &time);

    /// Установить дату сервера (формат строки "dd.mm.yyyy")
    void setDate(const std::string &date);

    /// Установить дату и время сервера (формат строки "dd.mm.yyyy hh:mm:ss")
    void setDateTime(const std::string &date_time);
};

#endif
