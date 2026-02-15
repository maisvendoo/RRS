#include    "switch.h"
#include    "switch-state.h"
#include    "trajectory.h"

#include    <filesystem.h>
#include    <Journal.h>

//#include    <math-funcs.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch::Switch(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch::~Switch()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory *Switch::getNextTraj(dir_t &dir) const
{
    if (dir == FWD)
    {
        if (state_fwd > 0)
        {
            // Направление вперёд, стрелка в плюсовом положении
            const Switch_way_t way = SW_FWD_PLUS;
            // Заменяем ориентацию траектории
            dir = static_cast<dir_t>(dir * orientations[way]);
            // Возвращаем указатель на траекторию
            return trajectories[way];
        }
        if (state_fwd < 0)
        {
            // Направление вперёд, стрелка в минусовом положении
            const Switch_way_t way = SW_FWD_MINUS;
            dir = static_cast<dir_t>(dir * orientations[way]);
            return trajectories[way];
        }
    }
    if (dir == BWD)
    {
        if (state_bwd > 0)
        {
            // Направление назад, стрелка в плюсовом положении
            const Switch_way_t way = SW_BWD_PLUS;
            dir = static_cast<dir_t>(dir * orientations[way]);
            return trajectories[way];
        }
        if (state_bwd < 0)
        {
            // Направление назад, стрелка в минусовом положении
            const Switch_way_t way = SW_BWD_MINUS;
            dir = static_cast<dir_t>(dir * orientations[way]);
            return trajectories[way];
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory* Switch::get_fwd_minus_traj() const
{
    return trajectories[SW_FWD_MINUS];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory* Switch::get_fwd_plus_traj() const
{
    return trajectories[SW_FWD_PLUS];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory* Switch::get_bwd_minus_traj() const
{
    return trajectories[SW_BWD_MINUS];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory* Switch::get_bwd_plus_traj() const
{
    return trajectories[SW_BWD_PLUS];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
dir_t Switch::getTrajOrientation(const Trajectory *traj)
{
    for (const Switch_way_t& way : switch_ways_t)
    {
        if (trajectories[way] == traj)
        {
            return orientations[way];
        }
    }
    return FWD;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::configure(CfgReader &cfg, QDomNode secNode, traj_list_t &traj_list)
{
    cfg.getString(secNode, "Name", name);
    Journal::instance()->info("Switch " + name + " will be initialized...");

    for (const traj_xml_nodes_t& txn : traj_xml_nodes)
    {
        QString traj_name;
        if (cfg.getString(secNode, txn.normal_trajectory_node_name, traj_name))
        {
            trajectories[txn.way] = traj_list.value(traj_name, nullptr);
            orientations[txn.way] = FWD;
        }

        if (trajectories[txn.way] == nullptr)
        {
            if (cfg.getString(secNode, txn.reversed_trajectory_node_name, traj_name))
            {
                trajectories[txn.way] = traj_list.value(traj_name, nullptr);
                orientations[txn.way] = BWD;
            }

            if (trajectories[txn.way] == nullptr)
            {
                Journal::instance()->info(txn.normal_trajectory_node_name + ": NONE");
            }
            else
            {
                Journal::instance()->info(txn.reversed_trajectory_node_name + ": " + trajectories[txn.way]->getName());
            }
        }
        else
        {
            Journal::instance()->info(txn.normal_trajectory_node_name + ": " + trajectories[txn.way]->getName());
        }
    }

    for (const Switch_way_t& way : switch_fwd_ways_t)
    {
        if (trajectories[way])
        {
            trajectories[way]->setBwdSwitch(this);
        }
    }

    for (const Switch_way_t& way : switch_bwd_ways_t)
    {
        if (trajectories[way])
        {
            trajectories[way]->setFwdSwitch(this);
        }
    }

    if (trajectories[SW_BWD_PLUS] == nullptr)
    {
        if (trajectories[SW_BWD_MINUS] == nullptr)
        {
            Journal::instance()->error("Switch " + name + " hasn't incoming trajectories!!!");
            state_bwd = NO_POSSIBLE_DIRECTION;
            ref_state_bwd = NO_POSSIBLE_DIRECTION;
        }
        else
        {
            Journal::instance()->info("Only one incoming trajectory (as minus way)");
            state_bwd = ONLY_MINUS;
            ref_state_bwd = ONLY_MINUS;
        }
    }
    else
    {
        if (trajectories[SW_BWD_MINUS] == nullptr)
        {
            Journal::instance()->info("Only one incoming trajectory (as plus way)");
            state_bwd = ONLY_PLUS;
            ref_state_bwd = ONLY_PLUS;
        }
        else
        {
            state_bwd = STATE_PLUS;

            int tmp_int = 0;
            if (cfg.getInt(secNode, "state_bwd", tmp_int))
            {
                if (tmp_int < 0)
                {
                    state_bwd = STATE_MINUS;
                    Journal::instance()->info("Incoming trajectories: 2. Switch is set to minus direction");
                }
                else
                {
                    Journal::instance()->info("Incoming trajectories: 2. Switch is set to plus direction");
                }
            }
            else
            {
                Journal::instance()->info("Incoming trajectories: 2. Parameter <state_bwd> not found, switch is set to plus direction");
            }
            ref_state_bwd = state_bwd;
        }
    }

    if (trajectories[SW_FWD_PLUS] == nullptr)
    {
        if (trajectories[SW_FWD_MINUS] == nullptr)
        {
            Journal::instance()->error("Switch " + name + " hasn't outcoming trajectories!!!");
            state_fwd = NO_POSSIBLE_DIRECTION;
            ref_state_fwd = NO_POSSIBLE_DIRECTION;
        }
        else
        {
            Journal::instance()->info("Only one outcoming trajectory (as minus way)");
            state_fwd = ONLY_MINUS;
            ref_state_fwd = ONLY_MINUS;
        }
    }
    else
    {
        if (trajectories[SW_FWD_MINUS] == nullptr)
        {
            Journal::instance()->info("Only one outcoming trajectory (as plus way)");
            state_fwd = ONLY_PLUS;
            ref_state_fwd = ONLY_PLUS;
        }
        else
        {
            state_fwd = STATE_PLUS;

            int tmp_int = 0;
            if (cfg.getInt(secNode, "state_bwd", tmp_int))
            {
                if (tmp_int < 0)
                {
                    state_fwd = STATE_MINUS;
                    Journal::instance()->info("Outcoming trajectories: 2. Switch is set to minus direction");
                }
                else
                {
                    Journal::instance()->info("Outcoming trajectories: 2. Switch is set to plus direction");
                }
            }
            else
            {
                Journal::instance()->info("Outcoming trajectories: 2. Parameter <state_bwd> not found, switch is set to plus direction");
            }
            ref_state_fwd = state_fwd;
        }
    }

    // Загружаем модули
    // Находим названия модулей, которые есть в траекториях спереди или сзади
    QStringList devices_names;
    for (const Switch_way_t& way : switch_ways_t)
    {
        if (trajectories[way] == nullptr)
            continue;

        for (const auto* device : trajectories[way]->getTrajectoryDevices())
        {
            QString name = device->getName();
            if (!devices_names.contains(name))
                devices_names.push_back(name);
        }
    }

    if (devices_names.isEmpty())
        return;

    // Загружаем к коннектору модули с названием connector-<имя>
    FileSystem &fs = FileSystem::getInstance();
    for (auto device_name : devices_names)
    {
        QString conn_module = "connector-" + device_name;
        QString conn_path = QString(fs.getModulesDir().c_str()) +
                                     QDir::separator() +
                                     conn_module;
        ConnectorDevice* module = loadConnectorDevice(conn_path);

        if (module == nullptr)
        {
            Journal::instance()->error("Fail to load module " + conn_module + " for connector " + name);
            continue;
        }

        Journal::instance()->info(
            "Loaded module " + conn_module + ".dll for connector " + name);

        // Указываем модулю, что он относится к этому коннектору
        module->setConnector(this);

        // Настраиваем связи модулей траекторий и коннектора,
        // параллельно топологии
        auto link_module = [](ConnectorDevice* module, QString& device_name,
                              Trajectory* traj, bool is_switched)
        {
            if (traj == nullptr)
            {
                return;
            }

            for (auto* device_bwd : traj->getTrajectoryDevices())
            {
                QString bwd_name = device_bwd->getName();
                if (device_name == bwd_name)
                {
                    if (is_switched)
                    {
                        module->setBwdTrajectoryDevice(device_bwd);
                    }
                    device_bwd->setFwdConnectorDevice(module);
                    break;
                }
            }
        };

        link_module(module, device_name, trajectories[SW_FWD_PLUS], (state_fwd > 0));
        link_module(module, device_name, trajectories[SW_FWD_MINUS], (state_fwd < 0));
        link_module(module, device_name, trajectories[SW_BWD_PLUS], (state_bwd > 0));
        link_module(module, device_name, trajectories[SW_BWD_MINUS], (state_bwd < 0));

        // TODO конфигурирование?

        devices.push_back(module);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::step(double t, double dt)
{
    int prev_state_fwd = state_fwd;
    int prev_state_bwd = state_bwd;

    // Если возможны обе траектории вперёд, проверяем переключение стрелки
    if ((trajectories[SW_FWD_PLUS] != nullptr) && (trajectories[SW_FWD_MINUS] != nullptr))
    {
        // Если какая-то траектория спереди занята ПЕ ближе,
        // чем заданная дистанция, ставим стрелку в это направление,
        // а также сбрасываем маршрут диспетчерской централизации
        if (trajectories[SW_FWD_PLUS]->isBusy(0.0, lock_by_busy_distance))
        {
            state_fwd = IS_BUSY_PLUS;
            ref_state_fwd = STATE_PLUS;
            in_route_by_signal_fwd = nullptr;
        }
        else
        {
            if (trajectories[SW_FWD_MINUS]->isBusy(0.0, lock_by_busy_distance))
            {
                state_fwd = IS_BUSY_MINUS;
                ref_state_fwd = STATE_MINUS;
                in_route_by_signal_fwd = nullptr;
            }
            else
            {
                // Переключаем стрелку в требуемое положение
                state_fwd = ref_state_fwd;
            }
        }
    }

    // Если возможны обе траектории назад, проверяем переключение стрелки
    if ((trajectories[SW_BWD_PLUS] != nullptr) && (trajectories[SW_BWD_MINUS] != nullptr))
    {
        // Если какая-то траектория сзади занята ПЕ ближе,
        // чем заданная дистанция, ставим стрелку в это направление,
        // а также сбрасываем маршрут диспетчерской централизации
        double len = trajectories[SW_BWD_PLUS]->getLength();
        if (trajectories[SW_BWD_PLUS]->isBusy(len - lock_by_busy_distance, len))
        {
            state_bwd = IS_BUSY_PLUS;
            ref_state_bwd = STATE_PLUS;
            in_route_by_signal_bwd = nullptr;
        }
        else
        {
            len = trajectories[SW_BWD_MINUS]->getLength();
            if (trajectories[SW_BWD_MINUS]->isBusy(len - lock_by_busy_distance, len))
            {
                state_bwd = IS_BUSY_MINUS;
                ref_state_bwd = STATE_MINUS;
                in_route_by_signal_bwd = nullptr;
            }
            else
            {
                // Переключаем стрелку в требуемое положение
                state_bwd = ref_state_bwd;
            }
        }
    }

    if ((prev_state_fwd != state_fwd) || (prev_state_bwd != state_bwd))
    {
        switch_state_t new_state;
        new_state.name = name;
        new_state.state_fwd = state_fwd;
        new_state.state_bwd = state_bwd;
        emit sendSwitchState(new_state.serialize());
    }

    // Переключаем связи модулей паралельно переключениям топологии
    int change_fwd = (sign(prev_state_fwd) != sign(state_fwd)) * sign(state_fwd);
    int change_bwd = (sign(prev_state_bwd) != sign(state_bwd)) * sign(state_bwd);
    for (auto device : devices)
    {
        if (change_fwd > 0)
        {
            bool no_change = true;
            for (auto* device_fwd : trajectories[SW_FWD_PLUS]->getTrajectoryDevices())
            {
                if (device->getName() == device_fwd->getName())
                {
                    device->setFwdTrajectoryDevice(device_fwd);
                    no_change = false;
                    break;
                }
            }
            if (no_change)
                device->setFwdTrajectoryDevice(nullptr);
        }
        if (change_fwd < 0)
        {
            bool no_change = true;
            for (auto* device_fwd : trajectories[SW_FWD_MINUS]->getTrajectoryDevices())
            {
                if (device->getName() == device_fwd->getName())
                {
                    device->setFwdTrajectoryDevice(device_fwd);
                    no_change = false;
                    break;
                }
            }
            if (no_change)
                device->setFwdTrajectoryDevice(nullptr);
        }

        if (change_bwd > 0)
        {
            bool no_change = true;
            for (auto* device_bwd : trajectories[SW_BWD_PLUS]->getTrajectoryDevices())
            {
                if (device->getName() == device_bwd->getName())
                {
                    device->setBwdTrajectoryDevice(device_bwd);
                    no_change = false;
                    break;
                }
            }
            if (no_change)
                device->setBwdTrajectoryDevice(nullptr);
        }
        if (change_bwd < 0)
        {
            bool no_change = true;
            for (auto* device_bwd : trajectories[SW_BWD_MINUS]->getTrajectoryDevices())
            {
                if (device->getName() == device_bwd->getName())
                {
                    device->setBwdTrajectoryDevice(device_bwd);
                    no_change = false;
                    break;
                }
            }
            if (no_change)
                device->setBwdTrajectoryDevice(nullptr);
        }
    }

    for (auto conn_device : devices)
    {
        conn_device->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Switch::serialize()
{
    QBuffer data;
    data.open(QIODevice::WriteOnly);
    QDataStream stream(&data);

    // Имя коннектора в буфер данных
    stream << name;

    // Сериализуем связанные с этим коннектором траектории
    for (const Switch_way_t& way : switch_ways_t)
    {
        serialize_connected_trajectory(stream, trajectories[way], orientations[way]);
    }

    // Помещаем в буФер состояние стрелки
    stream << state_fwd << state_bwd;

    return data.data();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::deserialize(QByteArray &data, traj_list_t &traj_list)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    // Извлекаем имя коннектора из буфера
    stream >> name;

    // Восстанавливаем связанные с этим коннектором траектории
    for (const Switch_way_t& way : switch_ways_t)
    {
        std::tie(trajectories[way], orientations[way]) = deserialize_connected_trajectory(stream, traj_list);
    }

    for (const Switch_way_t& way : switch_fwd_ways_t)
    {
        if (trajectories[way])
        {
            trajectories[way]->setBwdSwitch(this);
        }
    }

    for (const Switch_way_t& way : switch_bwd_ways_t)
    {
        if (trajectories[way])
        {
            trajectories[way]->setFwdSwitch(this);
        }
    }

    // Восстанавливаем статусы стрелки
    stream >> state_fwd;
    stream >> state_bwd;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::serialize_connected_trajectory(QDataStream &stream, Trajectory *traj, dir_t orient)
{
    // Анализирум наличие траектории на каждом из ответвлений,
    // пишем в буфер признак присутствия, и если она присутствует,
    // далее пишем имя этой траектории
    bool has_traj = (traj != nullptr);
    stream << has_traj;

    if (has_traj)
    {
        stream << orient;
        stream << traj->getName();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::pair<Trajectory*, dir_t> Switch::deserialize_connected_trajectory(QDataStream &stream,
                                                     traj_list_t &traj_list)
{
    // Извлекаем признак наличия траектории в этом направлении
    bool has_traj = false;
    stream >> has_traj;

    if (has_traj)
    {
        // Если она должна быть, восстанавливаем её ориентацию и имя
        dir_t orient;
        QString traj_name;
        stream >> orient;
        stream >> traj_name;

        // Если в списке траекторий есть такая, возвращаем указатель на нее
        if (traj_list.contains(traj_name))
        {
            return {traj_list[traj_name], orient};
        }
    }

    return {nullptr, FWD};
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::setStateFwd(Switch_state_t state)
{
    // Задаём стрелке состояние
    state_fwd = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::setStateBwd(Switch_state_t state)
{
    // Задаём стрелке состояние
    state_bwd = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::setRefStateFwd(Switch_state_t state)
{
    // Задаём стрелке требуемое направление
    ref_state_fwd = state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switch::setRefStateBwd(Switch_state_t state)
{
    // Задаём стрелке требуемое направление
    ref_state_bwd = state;
}
