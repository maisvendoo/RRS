#include    <scenario-manager.h>
#include    <Journal.h>
#include    <filesystem.h>
#include    <datetime.h>
#include    <route-segment.h>
#include    <switch-state.h>
#include    <signals-data-types.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ScenarioManager::ScenarioManager(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ScenarioManager::~ScenarioManager()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::init(const init_data_t &init_data)
{
    // Запоминаем данные инициализации, полученные от лаунчера
    launch_init_data = init_data;

    connect(delayTimer, &Timer::process, this, &ScenarioManager::slotDelayTimer);

    try
    {
        Journal::instance()->info("==== Lua Scenarios manager initialization ====");
        lua_init();
    }
    catch (const sol::error &error)
    {
        Journal::instance()->error(QString(error.what()));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ScenarioManager::run(const std::string &route_dir,
                          const std::string &scenario_name)
{
    // Полный путь к скрипту сценария
    FileSystem &fs = FileSystem::getInstance();
    std::string script_path = fs.getRouteRootDir() + fs.separator()
                              + route_dir + fs.separator()
                              + "scenarios" + fs.separator()
                              + scenario_name + fs.separator()
                              + "main.lua";

    try
    {
        Journal::instance()->info(QString("Starting script: %1...").arg(script_path.c_str()));
        lua.script_file(script_path);

        // Скрипт сработал, не вызвав исключения, поэтому мы играем сценарий
        is_scenario_active = true;
    }
    catch (const sol::error &error)
    {
        Journal::instance()->error(QString(error.what()));
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::processTasksQueue()
{
    // Если очередь задач не пуста
    if (!taskQueue.empty())
    {
        if (!delayTimer->isStarted())
        {
            // Получаем очередную задачу
            auto task = std::move(taskQueue.front());
            // Удаляем её из очереди
            taskQueue.pop();
            // Исполняем
            task();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::processTimeTriggers(const simulator_time_t &sim_time)
{
    // Очередь пуста - убегаем
    if (timeTriggerList.empty())
    {
        return;
    }

    // Нечего делать - перебираем все триггеры
    for (auto it = timeTriggerList.begin(); it != timeTriggerList.end();)
    {
        try
        {
            // Берем очередной
            auto trigger = *it;

            // Если настало его время
            if (isTimeHasCome(sim_time, trigger.action_time))
            {
                // Проверяем валидность прикрепленной к нему функции
                if (trigger.action_func.valid())
                {
                    // Исполняем эту функцию
                    trigger.action_func();

                    // Убиваем тригер, не будет больше такого момента времени
                    // и берем следующий
                    it = timeTriggerList.erase(it);

                    // уходим если нет больше триггеров
                    if (timeTriggerList.empty())
                    {
                        return;
                    }

                    continue;
                }
            }
        }
        catch (const sol::error &error)
        {
            Journal::instance()->error(QString("LUA: %1").arg(error.what()));
        }

        ++it;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ScenarioManager::isTimeHasCome(const simulator_time_t &start_time,
                                    const simulator_time_t &trig_time)
{
    if (start_time.date < trig_time.date)
    {
        // Завтра еще не наступило
        return false;
    }

    if (start_time.date > trig_time.date)
    {
        // Завтра не наступит никогда
        return false;
    }

    if (start_time.time >= trig_time.time)
    {
        // Время пришло
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskTimeTrigger(const simulator_time_t &trig_time,
                                      sol::function trigger_func)
{
    setTask([trig_time, trigger_func, this]{

        date_time_tirgger_t trigger;
        trigger.action_time = trig_time;
        trigger.action_func = trigger_func;

        this->timeTriggerList.push_back(trigger);
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setTimeTirgger(const std::string &time,
                                     sol::function trigger_func)
{
    if (time[0] == '-')
    {
        Journal::instance()->error("Set time trigger: You can't set trigger on a past time " + QString(time.c_str()));
        return;
    }

    if (time[0] != '+')
    {
        setAbsTimeTirgger(time, trigger_func);
    }
    else
    {
        setRelTimeTirgger(time, trigger_func);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setAbsTimeTirgger(const std::string &abs_time,
                                        sol::function trigger_func)
{
    // Определяем время начала игры
    simulator_time_t start_sim_time = simulator_time_t(launch_init_data.start_datetime);
    // Парсим заданное время
    server_time_t trig_time;
    strTimeToServerTime(abs_time, trig_time);

    simulator_time_t trig_date_time;

    // Если заданнный момент времени менее чем время старта
    if (start_sim_time.time >= trig_time)
    {
        // Значит ставим событие на следующий день
        server_date_t date = start_sim_time.date;
        date.nextDay();
        trig_date_time = simulator_time_t(date, trig_time);
    }
    else // Иначе, ставим на сегодня
    {
        trig_date_time = simulator_time_t(start_sim_time.date, trig_time);
    }

    taskTimeTrigger(trig_date_time, trigger_func);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setRelTimeTirgger(const std::string &rel_time,
                                        sol::function trigger_func)
{
    // Определяем время начала игры
    simulator_time_t start_sim_time = simulator_time_t(launch_init_data.start_datetime);
    // Парсим заданное время
    server_time_t trig_time;
    strTimeToServerTime(rel_time, trig_time);

    simulator_time_t trig_date_time = start_sim_time;
    // Переводим в секунды дельту времени
    double sec = trig_time.hour() * 3600 + trig_time.minute() * 60 + trig_time.sec();

    // Прибавляем эти секунды к времени старта
    if (trig_date_time.time.addTime(sec))
    {
        // если надо, добавляем еще день
        trig_date_time.date.nextDay();
    }

    // Ставим триггер
    taskTimeTrigger(trig_date_time, trigger_func);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::step(const simulator_time_t &sim_time, double dt)
{
    // Обработка очереди задач
    processTasksQueue();

    // Обработка временных триггеров
    processTimeTriggers(sim_time);

    delayTimer->step(sim_time.simulation_seconds, dt);

    curr_step = dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::addNewTrain(const scenario_train_data_t &train_data)
{
    // Если мы не играим по сценарию, то и незачем что-то делать
    if (!is_scenario_active)
    {
        return;
    }

    train_datas.push_back(train_data);

    // Сообщим вьюверу что появился новый поезд и надо
    // потроллить его машиниста транспарантом о переименовании
    emit sigSendTrainRenameRequire(train_data.getIndex());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setTrainIndex(size_t t_idx)
{
    if (t_idx >= train_datas.size())
    {
        return;
    }

    train_datas[t_idx].setIndex(t_idx);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string ScenarioManager::getTrainName(size_t t_idx)
{
    if (t_idx >= train_datas.size())
    {
        return "";
    }

    return train_datas[t_idx].name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setTrain(const scenario_train_data_t &train_data)
{
    init_data_t id = launch_init_data;
    id.train_config = QString(train_data.train_config.c_str());
    id.trajectory_name = QString(train_data.traj_name.c_str());
    id.init_coord = train_data.traj_coord;
    id.direction = train_data.direction;

    init_datas.push_back(id);

    train_datas.push_back(train_data);

    QString msg = QString("setTrain: %1 at traj=%2 coord=%3 dir=%4")
                      .arg(train_data.train_config.c_str())
                      .arg(train_data.traj_name.c_str())
                      .arg(train_data.traj_coord, 10, 'f', 3)
                      .arg(train_data.direction);

    Journal::instance()->info(msg);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::strTimeToServerTime(const std::string &str_time,
                                             server_time_t &time)
{
    QStringList tokens = QString(str_time.c_str()).split(':');

    // Если число параметров менее двух - ошибка
    if (tokens.size() < 2)
    {
        Journal::instance()->error(QString("setTime: Invalid time format: %1").arg(str_time.c_str()));
        return;
    }

    bool isOk = false;
    uint8_t sec = 0;

    // Если число параметров более двух - заданы секунды, обрабатываем
    if (tokens.size() > 2)
    {
        sec = static_cast<uint8_t>(tokens[2].toInt(&isOk));

        if (!isOk)
        {
            Journal::instance()->error("setTime: Invalid seconds format: " + tokens[2]);
        }
        else if (sec > 59)
        {
            Journal::instance()->error(QString("setTime: Seconds out of range: %1").arg(sec, 2));
            sec = 0;
        }
    }

    uint8_t hour = static_cast<uint8_t>(tokens[0].toInt(&isOk));

    if (!isOk)
    {
        Journal::instance()->error("setTime: Invalid hour format: " + tokens[0]);
        return;
    }

    if (hour > 23)
    {
        Journal::instance()->error(QString("setTime: Hour out of range: %1").arg(hour, 2));
        return;
    }

    uint8_t min = static_cast<uint8_t>(tokens[1].toInt(&isOk));

    if (!isOk)
    {
        Journal::instance()->error("setTime: Invalid minutes format: " + tokens[1]);
        return;
    }

    if (min > 59)
    {
        Journal::instance()->error(QString("setTime: Minutes out of range: %1").arg(min, 2));
        return;
    }

    time = server_time_t(hour, min, sec, 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setTime(const std::string &time)
{
    simulator_time_t sim_time = simulator_time_t(launch_init_data.start_datetime);
    server_time_t start_time;

    strTimeToServerTime(time, start_time);

    launch_init_data.start_datetime = simulator_time_t(sim_time.date, start_time).data();

    Journal::instance()->info("setTime: Time initialized at " + start_time.getString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::strDateToServerDate(const std::string &str_date,
                                          server_date_t &date)
{
    QStringList tokens = QString(str_date.c_str()).split('.');

    if (tokens.size() < 2)
    {
        Journal::instance()->error(QString("setDate: Invalid date format: %1").arg(str_date.c_str()));
        return;
    }

    bool isOk = false;
    uint16_t year = 0;
    simulator_time_t sim_time = simulator_time_t(launch_init_data.start_datetime);

    if (tokens.size() > 2)
    {
        year = static_cast<uint16_t>(tokens[2].toInt(&isOk));

        if (!isOk)
        {
            Journal::instance()->error("setDate: Invalid year format: " + tokens[2]);
        }
        else if (year == 0)
        {
            Journal::instance()->error(QString("setDate: Year out of range: %1").arg(year, 4));
            year = sim_time.date.year();
        }
    }

    // Дешифруем и проверяем корректность месяца
    uint8_t month = static_cast<uint8_t>(tokens[1].toInt(&isOk));

    if (!isOk)
    {
        Journal::instance()->error("setDate: Invalid month format: " + tokens[1]);
        return;
    }

    if ( (month < 1) || (month > 12) )
    {
        Journal::instance()->error(QString("setDate: Month out of range: %1").arg(month, 2));
        return;
    }

    // Дешифруем и проверяем корректность дня
    uint8_t day = static_cast<uint8_t>(tokens[0].toInt(&isOk));

    if (!isOk)
    {
        Journal::instance()->error("setDate: Invalid day format: " + tokens[0]);
        return;
    }

    uint8_t max_day = 0;

    // Проверяем максимальное число дней в том месяце, что пытаемся задать
    if (server_date_t().isLeapYear(year))
    {
        max_day = days_in_month_leap[month];
    }
    else
    {
        max_day = days_in_month_nleap[month];
    }

    if ( (day < 1) || (day > max_day))
    {
        Journal::instance()->error(QString("setDate: day out of range: %1").arg(day, 2));
        return;
    }

    date = server_date_t(year, month, day);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setDate(const std::string &date)
{
    simulator_time_t sim_time = simulator_time_t(launch_init_data.start_datetime);

    server_date_t start_date;
    strDateToServerDate(date, start_date);

    launch_init_data.start_datetime = simulator_time_t(start_date, sim_time.time).data();

    Journal::instance()->info("setDate: Time initialized at " + start_date.getString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setDateTime(const std::string &date_time)
{
    QStringList tokens = QString(date_time.c_str()).split(" ");

    if (tokens.size() < 2)
    {
        Journal::instance()->error(QString("setDateTime: Invalid date and time string format: %1").arg(date_time.c_str()));
        return;
    }

    setDate(tokens[0].toStdString());

    setTime(tokens[1].toStdString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::switchFwd(const std::string &switch_name)
{
    switch_state_t sw_state;
    sw_state.name = QString(switch_name.c_str());

    // Запрашиваем текущее состояние стрелки
    QByteArray switch_data = sw_state.serialize();
    emit sigGetSwitchState(switch_data);

    sw_state.deserialize(switch_data);

    Journal::instance()->info(QString("switchFwd: switch %1 state: %2")
                                  .arg(sw_state.name)
                                  .arg(sw_state.state_fwd, 2));

    if (sw_state.state_fwd != 0)
    {
        std::int8_t ref_state = -sw_state.state_fwd;
        QByteArray sc_data = switch_command_t({sw_state.name, 1, ref_state}).serialize();
        emit sigSwitchCommand(sc_data);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskSwitchFwd(const std::string &switch_name)
{
    setTask([switch_name, this]{ this->switchFwd(switch_name); });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::switchBwd(const std::string &switch_name)
{
    switch_state_t sw_state;
    sw_state.name = QString(switch_name.c_str());

    // Запрашиваем текущее состояние стрелки
    QByteArray switch_data = sw_state.serialize();
    emit sigGetSwitchState(switch_data);

    sw_state.deserialize(switch_data);

    Journal::instance()->info(QString("switchBwd: switch %1 state: %2")
                                  .arg(sw_state.name)
                                  .arg(sw_state.state_bwd, 2));

    if (sw_state.state_bwd != 0)
    {
        std::int8_t ref_state = -sw_state.state_bwd;
        QByteArray sc_data = switch_command_t({sw_state.name, -1, ref_state}).serialize();
        emit sigSwitchCommand(sc_data);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskSwitchBwd(const std::string &switch_name)
{
    setTask([switch_name, this]{ this->switchBwd(switch_name); });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::openSignal(const std::string& conn_name, int dir, bool for_train, bool for_shunting)
{
    signal_command_t sc = signal_command_t();
    sc.conn_name = QString(conn_name.c_str());
    sc.sig_dir = dir;
    if (for_train)
    {
        sc.command_open_train = true;
    }
    else
    {
        if (for_shunting)
        {
            sc.command_open_shunting = true;
        }
        else
        {
            sc.command_open_call = true;
        }
    }

    QByteArray sc_data = sc.serialize();
    emit sigSignalCommand(sc_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskOpenSignal(const std::string& conn_name, int dir, bool for_train, bool for_shunting)
{
    setTask([conn_name, dir, for_train, for_shunting, this]{
        this->openSignal(conn_name, dir, for_train, for_shunting);
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::closeSignal(const std::string &conn_name, int dir)
{
    signal_command_t sc = signal_command_t();
    sc.conn_name = QString(conn_name.c_str());
    sc.sig_dir = dir;
    sc.command_close = true;

    QByteArray sc_data = sc.serialize();
    emit sigSignalCommand(sc_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskCloseSignal(const std::string &conn_name, int dir)
{
    setTask([conn_name, dir, this]{ this->closeSignal(conn_name, dir); });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskSetDelay(double timeout)
{
    setTask([timeout, this]{
        this->delayTimer->setTimeout(timeout);
        this->delayTimer->start();
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::buildRoute(const std::string &start_traj, const std::string &target_traj, int dir)
{
    route_command_t rc;
    rc.trajectory_begin = QString(start_traj.c_str());
    rc.trajectory_end = QString(target_traj.c_str());
    rc.dir = (dir < 0) ? -1 : 1;

    QByteArray rc_data = rc.serialize();
    emit sigBuildRoute(rc_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskBuildRoute(const std::string &start_traj, const std::string &target_traj, int dir)
{
    setTask([start_traj, target_traj, dir, this]{
        this->buildRoute(start_traj, target_traj, dir);
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int ScenarioManager::findTrainByName(const std::string &name)
{
    for (auto& train_data : train_datas)
    {
        if (train_data.name == name)
        {
            return train_data.getIndex();
        }
    }

    return -1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string ScenarioManager::findTrainByIndex(int train_idx)
{
    for (auto& train_data : train_datas)
    {
        if (train_data.getIndex() == train_idx)
        {
            return train_data.name;
        }
    }

    return "";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::process_pos_triggers(std::string train_name,
                                           std::string traj_name,
                                           bool is_busy)
{
    // Нет тригеров - нечего ловить, уходим
    if (triggerList.empty())
    {
        return;
    }

    for (auto it = triggerList.begin(); it != triggerList.end();)
    {
        try
        {
            auto trigger = *it;
            if (trigger.valid())
            {
                auto result = trigger(train_name, traj_name, is_busy);
                if (result.valid() && result.get<bool>())
                {
                    it = triggerList.erase(it); // erase возвращает следующий итератор
                    if (triggerList.empty())
                        return;
                    continue; // не делаем ++it
                }
            }
        }
        catch (const sol::error &error)
        {
            Journal::instance()->error(QString("LUA: %1").arg(error.what()));
        }

        ++it; // инкрементируем только если не удаляли
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskSetPositionTrigger(sol::function trigger)
{
    setTask([trigger, this]{
        this->triggerList.push_back(trigger);
    });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::slotDelayTimer()
{
    delayTimer->stop();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::slotSetOpenSignalsQueue(QStringList conn_list, int dir, bool for_train, bool for_shunting)
{
    const double signal_open_first_delay = 0.75;
    taskSetDelay(signal_open_first_delay);

    const double signal_open_delay = 0.25;
    for (auto& conn_name : conn_list)
    {
        taskOpenSignal(conn_name.toStdString(), dir, for_train, for_shunting);
        taskSetDelay(signal_open_delay);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::slotRenameTrain(int train_idx, QString new_name)
{
    // Проверяем уникальность имени поезда
    if (findTrainByName(new_name.toStdString()) != -1)
    {
        emit sigSendExistedNameFound(train_idx);
        return;
    }

    for (size_t i = 0; i < train_datas.size(); ++i)
    {
        if (train_datas[i].getIndex() == train_idx)
        {
            train_datas[i].name = new_name.toStdString();
            break;
        }
    }

    emit sigRenameTrainInModel(train_idx, new_name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::slotChangeTrajStateByTrain(int train_idx, bool is_busy, QString traj_name)
{
    std::string train_name = findTrainByIndex(train_idx);

    if (is_busy)
    {
        process_pos_triggers(train_name, traj_name.toStdString(), is_busy);
    }
    else
    {
        process_pos_triggers(train_name, traj_name.toStdString(), is_busy);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setTask(task_t task)
{
    taskQueue.push(std::move(task));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::cpp_types_registration()
{
    // Регистрируем структуру данных о поездах
    lua.new_usertype<scenario_train_data_t>("TrainData",
                                            "name", &scenario_train_data_t::name,
                                            "config", &scenario_train_data_t::train_config,
                                            "traj", &scenario_train_data_t::traj_name,
                                            "coord", &scenario_train_data_t::traj_coord,
                                            "dir", &scenario_train_data_t::direction);

    Journal::instance()->info("TrainData => scenario_train_data_t binding...OK");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::sys_functions_registration()
{
    lua["setTrain"] = [this](const scenario_train_data_t &train_data) {
        this->setTrain(train_data);
    };

    Journal::instance()->info("setTrain method binding...OK");

    lua["setTime"] = [this](const std::string &time) {
        this->setTime(time);
    };

    Journal::instance()->info("setTime method binding...OK");

    lua["setDate"] = [this](const std::string &date) {
        this->setDate(date);
    };

    Journal::instance()->info("setDate method binding...OK");

    lua["setDateTime"] = [this](const std::string &date_time) {
        this->setDateTime(date_time);
    };

    Journal::instance()->info("setDateTime method binding...OK");

    lua["switchFwd"] = [this](const std::string &switch_name) {
        this->taskSwitchFwd(switch_name);
    };

    Journal::instance()->info("switchFwd method binding...OK");

    lua["switchBwd"] = [this](const std::string &switch_name) {
        this->taskSwitchBwd(switch_name);
    };

    Journal::instance()->info("switchBwd method binding...OK");

    lua.set_function("openSignal",
                     sol::overload(
                         [this](const std::string &conn_name, int dir) {
                             this->taskOpenSignal(conn_name, dir);
                         },
                         [this](const std::string &conn_name) {
                             this->taskOpenSignal(conn_name);
                         }
                         ));

    Journal::instance()->info("openSignal method binding...OK");

    lua.set_function("closeSignal",
                     sol::overload(
                         [this](const std::string &conn_name, int dir) {
                             this->taskCloseSignal(conn_name, dir);
                         },
                         [this](const std::string &conn_name) {
                             this->taskCloseSignal(conn_name);
                         }
                         ));

    Journal::instance()->info("closeSignal method binding...OK");

    lua["delay"] = [this](double timeout) {
        this->taskSetDelay(timeout);
    };

    Journal::instance()->info("delay method binding...OK");

    lua["buildRoute"] = [this](const std::string &start_traj, const std::string &target_traj, int dir) {
        this->taskBuildRoute(start_traj, target_traj, dir);
    };

    Journal::instance()->info("buildRoute method binding...OK");

    lua.set_function("setTrigger", [&](sol::function trigger) {
        taskSetPositionTrigger(trigger);
    });

    Journal::instance()->info("setTrigger method binding...OK");

    lua.set_function("setAbsTimeTrigger", [&](const std::string &abs_time, sol::function trigger_func){
        this->setTimeTirgger(abs_time, trigger_func);
    });

    Journal::instance()->info("setAbsTimeTrigger method binding...OK");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::lua_debug_init()
{
    lua_dbg->init(lua);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::lua_libraries_init()
{
    lua.open_libraries(sol::lib::base,
                       sol::lib::package,
                       sol::lib::debug,
                       sol::lib::io,
                       sol::lib::table,
                       sol::lib::string,
                       sol::lib::coroutine);

    lua["package"]["path"] = lua["package"]["path"].get<std::string>() + ";../modules/lua/?.lua";

    // Подключаем свои библиотеки
    lua.script(R"(
        -- Фабрика триггеров под типовые задачи
        local trig_mod = require('triggers_fabric')
    )");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::lua_init()
{
    // Инициализация библиотек Lua
    lua_libraries_init();

    // Регистрация C++ типов в интерпретаторе
    cpp_types_registration();

    // Регистрация системных функций
    sys_functions_registration();

    // Инициализация отладки
    if (launch_init_data.lua_debug)
    {
        lua_debug_init();
    }

    // Псеводонимы для значений, возвращаемых триггерами,
    // показвающие, оставить ли триггер в очереди, или удалить его
    lua.script(R"(
        TRIG_DELETE = true
        TRIG_SAFE = false
    )");
}
