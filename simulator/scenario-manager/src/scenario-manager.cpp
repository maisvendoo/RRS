#include    <scenario-manager.h>
#include    <Journal.h>
#include    <datetime.h>

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

    try
    {
        Journal::instance()->info("==== Lua Scenarios manager initialization ====");
        types_registration();
    }
    catch (const sol::error &error)
    {
        Journal::instance()->error(QString(error.what()));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ScenarioManager::run(const std::string &script_path)
{
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
void ScenarioManager::setTrain(const scenario_train_data_t &train_data)
{
    init_data_t id = launch_init_data;
    id.train_config = QString(train_data.train_file.c_str());
    id.trajectory_name = QString(train_data.traj_name.c_str());
    id.init_coord = train_data.traj_coord;
    id.direction = train_data.direction;

    init_datas.push_back(id);

    QString msg = QString("setTrain: %1 at traj=%2 coord=%3 dir=%4")
                      .arg(train_data.train_file.c_str())
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

        if (sec > 59)
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
void ScenarioManager::types_registration()
{
    lua.open_libraries(sol::lib::base);

    // Регистрируем структуру данных о поездах
    lua.new_usertype<scenario_train_data_t>("TrainData",
                                            "name", &scenario_train_data_t::train_file,
                                            "traj", &scenario_train_data_t::traj_name,
                                            "coord", &scenario_train_data_t::traj_coord,
                                            "dir", &scenario_train_data_t::direction);

    Journal::instance()->info("TrainData => scenario_train_data_t binding...OK");

    // Пробраcываем наши методы
    lua["setTrain"] = [this](const scenario_train_data_t &train_data) {
        this->setTrain(train_data);
    };

    lua["setTime"] = [this](const std::string &time) {
        this->setTime(time);
    };

    Journal::instance()->info("setTrain method binding...OK");
}
