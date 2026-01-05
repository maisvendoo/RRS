#include    <scenario-manager.h>
#include    <Journal.h>

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

    QString msg = QString("Set train: %1 at traj=%2 coord=%3 dir=%4")
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
