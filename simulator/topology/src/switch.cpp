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

    struct traj_xml_nodes_t
    {
        Switch_way_t way;
        QString normal_trajectory_node_name;
        QString reversed_trajectory_node_name;
        traj_xml_nodes_t(Switch_way_t w, QString n, QString r)
            : way(w)
            , normal_trajectory_node_name(n)
            , reversed_trajectory_node_name(r){}
    };
    const traj_xml_nodes_t traj_xml_nodes[] =
    {
        traj_xml_nodes_t(SW_FWD_PLUS, QString("fwdPlusTraj"), QString("fwdPlusTrajReversed")),
        traj_xml_nodes_t(SW_FWD_MINUS, QString("fwdMinusTraj"), QString("fwdMinusTrajReversed")),
        traj_xml_nodes_t(SW_BWD_PLUS, QString("bwdPlusTraj"), QString("bwdPlusTrajReversed")),
        traj_xml_nodes_t(SW_BWD_MINUS, QString("bwdMinusTraj"), QString("bwdMinusTrajReversed"))
    };

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
            dir_t dir = static_cast<dir_t>(BWD * orientations[way]);
            if (Switch* sw = trajectories[way]->getNextSwitch(dir))
            {
                Journal::instance()->error("Switch " + name + " has outcoming trajectory " + trajectories[way]->getName());
                Journal::instance()->error("but this trajectory already connected to switch " + sw->getName());
                trajectories[way] = nullptr;
            }
            else
            {
                if (dir == BWD)
                    trajectories[way]->setBwdSwitch(this);
                else
                    trajectories[way]->setFwdSwitch(this);
            }
        }
    }

    for (const Switch_way_t& way : switch_bwd_ways_t)
    {
        if (trajectories[way])
        {
            dir_t dir = static_cast<dir_t>(FWD * orientations[way]);
            if (Switch* sw = trajectories[way]->getNextSwitch(dir))
            {
                Journal::instance()->error("Switch " + name + " has incoming trajectory " + trajectories[way]->getName());
                Journal::instance()->error("but this trajectory already connected to switch " + sw->getName());
                trajectories[way] = nullptr;
            }
            else
            {
                if (dir == FWD)
                    trajectories[way]->setFwdSwitch(this);
                else
                    trajectories[way]->setBwdSwitch(this);
            }
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
            if (cfg.getInt(secNode, "state_fwd", tmp_int))
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
        auto link_module = [](ConnectorDevice* module, QString& conn_device_name,
                              Trajectory* traj, std::int8_t dir, std::int8_t orient,
                              bool is_switched)
        {
            if (traj == nullptr)
            {
                return;
            }

            for (auto* device : traj->getTrajectoryDevices())
            {
                QString traj_device_name = device->getName();
                if (conn_device_name == traj_device_name)
                {
                    if (is_switched)
                    {
                        module->setTrajectoryDevice(device, dir, orient);
                    }
                    device->setConnectorDevice(module, -(dir * orient));
                    break;
                }
            }
        };

        link_module(module, device_name, trajectories[SW_FWD_PLUS], 1, orientations[SW_FWD_PLUS], (state_fwd > 0));
        link_module(module, device_name, trajectories[SW_FWD_MINUS], 1, orientations[SW_FWD_MINUS], (state_fwd < 0));
        link_module(module, device_name, trajectories[SW_BWD_PLUS], -1, orientations[SW_BWD_PLUS], (state_bwd > 0));
        link_module(module, device_name, trajectories[SW_BWD_MINUS], -1, orientations[SW_BWD_MINUS], (state_bwd < 0));

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

    auto check_busy = [](Trajectory* traj, bool from_begin_or_end, double distance) -> bool
    {
        if (from_begin_or_end)
        {
            return traj->isBusy(0.0, distance);
        }

        double len = traj->getLength();
        return traj->isBusy(len - distance, len);
    };

    // Если возможны обе траектории вперёд, проверяем переключение стрелки
    if ((trajectories[SW_FWD_PLUS] != nullptr) && (trajectories[SW_FWD_MINUS] != nullptr))
    {
        // Если какая-то траектория спереди занята ПЕ ближе,
        // чем заданная дистанция, ставим стрелку в это направление,
        // а также сбрасываем маршрут диспетчерской централизации
        if (check_busy(trajectories[SW_FWD_PLUS], (orientations[SW_FWD_PLUS] == FWD), lock_by_busy_distance))
        {
            state_fwd = IS_BUSY_PLUS;
            ref_state_fwd = STATE_PLUS;
            in_route_by_signal_fwd = nullptr;
        }
        else
        {
            if (check_busy(trajectories[SW_FWD_MINUS], (orientations[SW_FWD_MINUS] == FWD), lock_by_busy_distance))
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
        if (check_busy(trajectories[SW_BWD_PLUS], (orientations[SW_BWD_PLUS] != FWD), lock_by_busy_distance))
        {
            state_bwd = IS_BUSY_PLUS;
            ref_state_bwd = STATE_PLUS;
            in_route_by_signal_bwd = nullptr;
        }
        else
        {
            if (check_busy(trajectories[SW_BWD_MINUS], (orientations[SW_BWD_MINUS] != FWD), lock_by_busy_distance))
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
    std::int8_t change_fwd = (sign(prev_state_fwd) != sign(state_fwd)) * sign(state_fwd);
    std::int8_t change_bwd = (sign(prev_state_bwd) != sign(state_bwd)) * sign(state_bwd);

    auto link_module = [](ConnectorDevice* module, Trajectory* traj,
                          std::int8_t dir, std::int8_t orient)
    {
        bool no_change = true;
        for (auto* device_fwd : traj->getTrajectoryDevices())
        {
            if (module->getName() == device_fwd->getName())
            {
                module->setTrajectoryDevice(device_fwd, dir, orient);
                no_change = false;
                break;
            }
        }
        if (no_change)
        {
            module->setTrajectoryDevice(nullptr, dir, orient);
        }
    };
    for (auto conn_device : devices)
    {
        if (change_fwd > 0)
        {
            Switch_way_t way = SW_FWD_PLUS;
            link_module(conn_device, trajectories[way], 1, orientations[way]);
        }
        if (change_fwd < 0)
        {
            Switch_way_t way = SW_FWD_MINUS;
            link_module(conn_device, trajectories[way], 1, orientations[way]);
        }

        if (change_bwd > 0)
        {
            Switch_way_t way = SW_BWD_PLUS;
            link_module(conn_device, trajectories[way], -1, orientations[way]);
        }
        if (change_bwd < 0)
        {
            Switch_way_t way = SW_BWD_MINUS;
            link_module(conn_device, trajectories[way], -1, orientations[way]);
        }

        // Шаг моделирования модулей путевой инфраструктуры
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
            dir_t dir = static_cast<dir_t>(BWD * orientations[way]);
            if (dir == BWD)
                trajectories[way]->setBwdSwitch(this);
            else
                trajectories[way]->setFwdSwitch(this);
        }
    }

    for (const Switch_way_t& way : switch_bwd_ways_t)
    {
        if (trajectories[way])
        {
            dir_t dir = static_cast<dir_t>(FWD * orientations[way]);
            if (dir == FWD)
                trajectories[way]->setFwdSwitch(this);
            else
                trajectories[way]->setBwdSwitch(this);
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
