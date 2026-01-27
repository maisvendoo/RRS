#ifndef     SCENARIO_MANAGER_H
#define     SCENARIO_MANAGER_H

#include    <datetime.h>
#include    <QObject>
#include    <queue>
#include    <sol/sol.hpp>

#include    <scenario-manager-export.h>
#include    <scenario-train-data.h>

#include    <init_data.h>
#include    <timer.h>

#include    <lua-debugger.h>
#include    <date-time-trigger.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using task_t = std::function<void()>;

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
    bool run(const std::string &route_dir, const std::string &script_path);

    /// Данные инициализации симулятора
    std::vector<init_data_t> init_datas;

    std::int64_t getStartDateTime() const
    {
        return launch_init_data.start_datetime;
    }

    /// Шаг симуляции (выполнение очереди задач)
    void step(const simulator_time_t &sim_time, double dt);

    std::vector<scenario_train_data_t> train_datas;

    void addNewTrain(const scenario_train_data_t &train_data);

    void setTrainIndex(size_t t_idx);

    std::string getTrainName(size_t t_idx);

    bool isTrainAutostarted(size_t t_idx);

signals:

    void sigGetSwitchState(QByteArray &switch_data);

    void sigSwitchCommand(QByteArray& switch_command);

    void sigSignalCommand(QByteArray& signal_data);

    void sigBuildTrainRoute(QByteArray& route_data);

    void sigBuildShuntingRoute(QByteArray& route_data);

    void sigSetSwitchsAlongRoute(QByteArray &route_data);

    /// Этот сигнал инициирует сообщение во вьювер,
    /// о необходимости задать имя поезда
    /// (вывесим транспарат, который будет назойтиво висеть, напоминая что надо дать имя поезду)
    void sigSendTrainRenameRequire(int train_idx);

    /// Сообщить через вьювер, что такое имя уже есть
    void sigSendExistedNameFound(int train_idx);

    /// Переименовать пеозд в модели
    void sigRenameTrainInModel(int train_idx, QString new_name);

private:

    /// Контекст интерпретатора Lua
    sol::state lua;

    /// Данные инициализации, полученные от лаунчера
    init_data_t launch_init_data;

    /// Очередь задач
    std::queue<task_t> taskQueue;

    /// Список позиционных триггеров
    std::vector<sol::protected_function> triggerList;

    /// Список временных триггеров
    std::vector<date_time_tirgger_t> timeTriggerList;

    /// Таймер задержки исполнения очереди задач
    Timer *delayTimer = new Timer(0.1, false);

    /// Текущий шаг
    double curr_step = 0.0;

    LuaDebugger *lua_dbg = new LuaDebugger;

    /// Флаг идентифицирующий исполнение сценария
    bool is_scenario_active = false;    

    /// Поставить задачу в очередь
    void setTask(task_t task);

    /// Инициализация интерпретатора Lua
    void lua_init();

    /// Регистрация типов C++
    void cpp_types_registration();

    /// Регистрация системных функций
    void sys_functions_registration();

    /// Инициализация отладки
    void lua_debug_init();

    /// Создать поезд игрока
    void setTrain(const scenario_train_data_t &train_data);

    /// Перевести строку в структуру времени сервера
    void strTimeToServerTime(const std::string &str_time,
                                server_time_t &time);

    /// Установить время сервера (формат строки "hh:mm:ss"
    void setTime(const std::string &time);

    /// Перевести строку в структуру даты сервера
    void strDateToServerDate(const std::string &str_date,
                             server_date_t &date);

    /// Установить дату сервера (формат строки "dd.mm.yyyy")
    void setDate(const std::string &date);

    /// Установить дату и время сервера (формат строки "dd.mm.yyyy hh:mm:ss")
    void setDateTime(const std::string &date_time);

    /// Переключить стрелку спереди
    void switchFwd(const std::string &switch_name);

    /// Установка задачи на переключение стрелки спереди
    void taskSwitchFwd(const std::string &switch_name);

    /// Переключить стрелку сзади
    void switchBwd(const std::string &switch_name);

    /// Установка задачи на переключение стрелки сзади
    void taskSwitchBwd(const std::string &switch_name);

    /// Открыть сигнал
    void openSignal(const std::string &conn_name, int dir, bool for_train = true, bool for_shunting = true);    

    /// Установка задачи открытия сигнала
    void taskOpenSignal(const std::string &conn_name, int dir = 1, bool for_train = true, bool for_shunting = true);

    /// Закрыть сигнал
    void closeSignal(const std::string &conn_name, int dir);

    /// Установка задачи закрытия сигнала
    void taskCloseSignal(const std::string &conn_name, int dir = 1);

    /// Установка задержки исполнения очереди
    void taskSetDelay(double timeout);

    /// Задать маршрут
    void buildRoute(const std::string &start_traj, const std::string &target_traj, int dir, bool is_train);

    /// Установить стрелки по маршруту
    void setSwitchsAlongRoute(const std::string &start_traj, const std::string &target_traj, int dir);

    /// Установить стрелки по маршруту - соответсвующая задача
    void taskSetSwitchsAlongRoute(const std::string &start_traj, const std::string &target_traj, int dir);

    /// Установка задачи задания маршрута
    void taskBuildRoute(const std::string &start_traj, const std::string &target_traj, int dir, bool is_train);

    /// Найти индекc поезда по имени
    int findTrainByName(const std::string &name);

    /// Найти имя поезда по индексу
    std::string findTrainByIndex(int train_idx);

    /// Обработка позиционных триггеров
    void process_pos_triggers(std::string train_name, std::string traj_name, bool is_busy);

    /// Установить позиционный триггер
    void taskSetPositionTrigger(sol::function trigger);

    /// Инициализация библиотек Lua
    void lua_libraries_init();

    /// Обработка очереди задач
    void processTasksQueue();

    /// Обработка очереди временных триггеров
    void processTimeTriggers(const simulator_time_t &sim_time);

    /// Определение когда настало заданное в триггере время (с учетом даты)
    bool isTimeHasCome(const simulator_time_t &start_time, const simulator_time_t &trig_time);

    /// Установить временной триггер по абсолютным дате и времени (общая системная задача,
    /// недоступная из Lua)
    void taskTimeTrigger(const simulator_time_t &trig_time, sol::function trigger_func);

    /// Установить триггер на время
    void setTimeTirgger(const std::string &time, sol::function trigger_func);

    void setAbsTimeTirgger(const std::string &abs_time, sol::function trigger_func);

    void setRelTimeTirgger(const std::string &rel_time, sol::function trigger_func);

private slots:

    void slotDelayTimer();

public slots:

    /// Построение очереди задач на открытие сигналов
    void slotSetOpenSignalsQueue(QStringList conn_list, int dir, bool for_train, bool for_shunting);

    /// Переименование поезда по команде от сервера, принявшего новое имя
    void slotRenameTrain(int train_idx, QString new_name);

    /// Обработка события занятости или освобождения какой-либо траектории
    /// на карте
    void slotChangeTrajStateByTrain(int train_idx, bool is_busy, QString traj_name);
};

#endif
