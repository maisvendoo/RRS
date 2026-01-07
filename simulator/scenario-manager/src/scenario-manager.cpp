#include    <scenario-manager.h>
#include    <Journal.h>
#include    <filesystem.h>
#include    <datetime.h>
#include    <switch-state.h>

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
void ScenarioManager::step(double t, double dt)
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

    delayTimer->step(t, dt);
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
void ScenarioManager::setTime(const std::string &time)
{
    QStringList tokens = QString(time.c_str()).split(':');

    // Если число параметров менее двух - ошибка
    if (tokens.size() < 2)
    {
        Journal::instance()->error(QString("setTime: Invalid time format: %1").arg(time.c_str()));
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

    simulator_time_t sim_time = simulator_time_t(launch_init_data.start_datetime);
    server_time_t start_time = server_time_t(hour, min, sec, 0);
    launch_init_data.start_datetime = simulator_time_t(sim_time.date, start_time).data();

    Journal::instance()->info("setTime: Time initialized at " + start_time.getString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::setDate(const std::string &date)
{
    QStringList tokens = QString(date.c_str()).split('.');

    if (tokens.size() < 2)
    {
        Journal::instance()->error(QString("setDate: Invalid date format: %1").arg(date.c_str()));
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

    server_date_t start_date = server_date_t(year, month, day);
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
        sw_state.state_fwd = -sw_state.state_fwd;

        switch_data = sw_state.serialize();
        emit sigSetSwitchState(switch_data);
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
        sw_state.state_bwd = -sw_state.state_bwd;

        switch_data = sw_state.serialize();
        emit sigSetSwitchState(switch_data);
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
void ScenarioManager::openSignal(const std::string &conn_name, int dir)
{
    QByteArray signal_data;
    QBuffer buff(&signal_data);
    buff.open(QIODevice::WriteOnly);
    QDataStream stream(&buff);

    stream << QString(conn_name.c_str());
    stream << dir;

    emit sigOpenSignal(signal_data);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::taskOpenSignal(const std::string &conn_name, int dir)
{
    setTask([conn_name, dir, this]{ this->openSignal(conn_name, dir); });
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::closeSignal(const std::string &conn_name, int dir)
{
    QByteArray signal_data;
    QBuffer buff(&signal_data);
    buff.open(QIODevice::WriteOnly);
    QDataStream stream(&buff);

    stream << QString(conn_name.c_str());
    stream << dir;

    emit sigCloseSignal(signal_data);
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
void ScenarioManager::slotDelayTimer()
{
    delayTimer->stop();
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ScenarioManager::lua_init()
{
    lua.open_libraries(sol::lib::base, sol::lib::package);

    // Регистрация C++ типов в интерпретаторе
    cpp_types_registration();

    // Регистрация системных функций
    sys_functions_registration();
}
