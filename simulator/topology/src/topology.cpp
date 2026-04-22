#include "enter-signal.h"
#include "exit-signal.h"
#include "route-signal.h"
#include "shunting-signal.h"
#include "signal-command.h"
#include "station-signal.h"
#include    <vehicle.h>
#include    <vehicle-controller.h>
#include    <topology.h>

#include    <cassert>
#include    <QDir>
#include    <QDirIterator>
#include    <QFile>

#include    <CfgReader.h>
#include    <switch.h>
#include    <switch-state.h>
#include    <line-signal.h>

#include    <Journal.h>
#include    <filesystem.h>

#include    <queue>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Topology::Topology(QObject *parent) : QObject(parent)
{
    vehicle_control.clear();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Topology::~Topology() = default;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::load(QString route_dir, bool solve_errors)
{
    const FileSystem& fs = FileSystem::getInstance();

    const QString route_path = QString(fs.getRouteRootDir().c_str()) +
        QDir::separator() + route_dir;

    QStringList names = getTrajNamesList(route_path);

    if (names.isEmpty())
    {
        Journal::instance()->error("TRAJECTORIES NOT FOUND!!!");
        return false;
    }

    std::vector<std::vector<module_cfg_t>> all_modules = load_topology_configs(route_path);

    for (const QString& name : names)
    {
        Trajectory* traj = new Trajectory();

        std::vector<module_cfg_t> modules;
        for (const std::vector<module_cfg_t>& all_cfgs : all_modules)
        {
            for (const module_cfg_t& module_cfg : all_cfgs)
            {
                if (module_cfg.traj_names.contains(name))
                {
                    modules.push_back(module_cfg);
                    break;
                }
            }
        }

        if (traj->load(route_path, name, modules, solve_errors))
        {
            Journal::instance()->info("Loaded trajectory: " + name);
        }
        else
        {
            Journal::instance()->error("Can't load trajectory: " + name);
        }

        traj_list.insert(name, traj);
        connect(traj, &Trajectory::sendTrajBusyState, this, &Topology::sendTrajBusyState);
    }

    if (traj_list.empty())
    {
        Journal::instance()->error("Empty list of trajectories");
        return false;
    }

    load_topology(route_path);

    if (!load_stations(route_path))
    {
        Journal::instance()->error("Can't to load staions list");
    }

    get_route_name(route_path);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::addTrain(const topology_pos_t &tp, std::vector<Vehicle *> *vehicles)
{
    if (vehicles->empty())
    {
        Journal::instance()->error("Vehicles list is empty!!!");
        return false;
    }

    // Находим уазатель на стартовую траекторию
    Trajectory* cur_traj = traj_list.value(tp.traj_name, nullptr);
    if (cur_traj == nullptr)
    {
        Journal::instance()->critical(tp.traj_name + " INVALID INITIAL TRAJECTORY!!!");
        return false;
    }

    double train_length = 0.0;
    for (size_t i = 0; i < vehicles->size(); ++i)
    {
        train_length += (*vehicles)[i]->getLength();
    }

    double traj_coord = std::clamp(tp.traj_coord, 0.0, cur_traj->getLength());
    dir_t dir = (tp.dir > 0) ? FWD : BWD;

    // Проходим по стартовой и соседним траекториям на длину поезда.
    // Реализация похожа на Trajectory::findTrajectoryAtCoord(),
    // но здесь нужно проверить занятость и переключить стрелки в направлении поезда
    {
        double begin_coord = traj_coord;
        double end_coord = begin_coord - static_cast<double>(dir) * train_length;
        Trajectory* check_traj = cur_traj;
        dir_t move_dir;
        while (true)
        {
            if (begin_coord < end_coord)
            {
                if (check_traj->isBusy(begin_coord, end_coord))
                {
                    // Если этот участок траектории уже занят подвижным составом, выходим
                    Journal::instance()->critical(check_traj->getName() + " TRAJECTORY IS BUSY!!!");
                    return false;
                }
            }
            else
            {
                if (check_traj->isBusy(end_coord, begin_coord))
                {
                    // Если этот участок траектории уже занят подвижным составом, выходим
                    Journal::instance()->critical(check_traj->getName() + " TRAJECTORY IS BUSY!!!");
                    return false;
                }
            }

            if (end_coord < 0.0)
            {
                // Если траекторная координата меньше нуля - заехали за стрелку сзади
                move_dir = BWD;
            }
            else
            {
                if (end_coord > check_traj->getLength())
                {
                    // Если траекторная координата превысила длину траектории - заехали за стрелку спереди
                    move_dir = FWD;
                    // Учитываем вылет за пределы траектории
                    end_coord = end_coord - check_traj->getLength();
                }
                else
                {
                    // УРА! Находимся в пределах траектории: выходим из цикла,
                    // продолжаем инициализацию поезда на топологии
                    break;
                }
            }

            // Отслеживаем разворот ориентации траектории
            dir_t new_dir = move_dir;

            // Получаем указатель на стрелку в конце траектории
            Switch* next_sw = check_traj->getNextSwitch(new_dir);
            if (next_sw == nullptr)
            {
                // Если коннектора нет, выходим
                Journal::instance()->critical(check_traj->getName() + " TRAJECTORY WITH INVALID NEXT SWITCH!!!");
                return false;
            }

            // Проверяем стрелку на взрез
            if (new_dir > 0)
            {
                // Движемся через стрелку вперёд, значит проверяем траекторию сзади
                Switch_state_t bwd_state = next_sw->getStateBwd();
                if (check_traj == next_sw->trajectories[SW_BWD_PLUS])
                {
                    // Если пришли с прямой траектории, стрелка не должна быть в минусовом положении
                    if (bwd_state <= 0)
                    {
                        if (bwd_state != STATE_MINUS)
                        {
                            // Если стрелка занята в минусовом положении, выходим
                            Journal::instance()->critical(next_sw->getName() + " SWITCH IS BUSY!!!");
                            return false;
                        }
                        // Переводим стрелку в плюсовое положение
                        next_sw->setStateBwd(STATE_PLUS);
                        next_sw->setRefStateBwd(STATE_PLUS);
                    }
                }
                else
                {
                    if (check_traj == next_sw->trajectories[SW_BWD_MINUS])
                    {
                        // Если пришли с боковой траектории, стрелка не должна быть в плюсовом положении
                        if (bwd_state >= 0)
                        {
                            if (bwd_state != STATE_PLUS)
                            {
                                // Если стрелка занята в плюсовом положении, выходим
                                Journal::instance()->critical(next_sw->getName() + " SWITCH IS BUSY!!!");
                                return false;
                            }
                            // Переводим стрелку в минусовое положение
                            next_sw->setStateBwd(STATE_MINUS);
                            next_sw->setRefStateBwd(STATE_MINUS);
                        }
                    }
                    else
                    {
                        // Вокруг стрелки нет этой траектории,
                        // какая-то ошибка в топологии, на всякий случай выходим
                        Journal::instance()->critical(next_sw->getName() + " SWITCH WITH INVALID CONFIG!!!");
                        return false;
                    }
                }
            }
            else
            {
                // Движемся через стрелку назад, значит проверяем траекторию спереди
                Switch_state_t fwd_state = next_sw->getStateFwd();
                if (check_traj == next_sw->trajectories[SW_FWD_PLUS])
                {
                    // Если пришли с прямой траектории, стрелка не должна быть в минусовом положении
                    if (fwd_state <= 0)
                    {
                        if (fwd_state != STATE_MINUS)
                        {
                            // Если стрелка занята в минусовом положении, выходим
                            Journal::instance()->critical(next_sw->getName() + " SWITCH IS BUSY!!!");
                            return false;
                        }
                        // Переводим стрелку в плюсовое положение
                        next_sw->setStateFwd(STATE_PLUS);
                        next_sw->setRefStateFwd(STATE_PLUS);
                    }
                }
                else
                {
                    if (check_traj == next_sw->trajectories[SW_FWD_MINUS])
                    {
                        // Если пришли с боковой траектории, стрелка не должна быть в плюсовом положении
                        if (fwd_state >= 0)
                        {
                            if (fwd_state != STATE_PLUS)
                            {
                                // Если стрелка занята в плюсовом положении, выходим
                                Journal::instance()->critical(next_sw->getName() + " SWITCH IS BUSY!!!");
                                return false;
                            }
                            // Переводим стрелку в минусовое положение
                            next_sw->setStateFwd(STATE_MINUS);
                            next_sw->setRefStateFwd(STATE_MINUS);
                        }
                    }
                    else
                    {
                        // Вокруг стрелки нет этой траектории,
                        // какая-то ошибка в топологии, на всякий случай выходим
                        Journal::instance()->critical(next_sw->getName() + " SWITCH WITH INVALID CONFIG!!!");
                        return false;
                    }
                }
            }

            // Получаем указатель на ту траекторию, с которой нас соединяет стрелка
            check_traj = next_sw->getNextTraj(new_dir);

            // Если за стрелкой нет траектории, выходим
            if (check_traj == nullptr)
            {
                Journal::instance()->critical(next_sw->getName() + " SWITCH WITH INVALID NEXT TRAJECTORY!!!");
                return false;
            }

            // Обновляем текущую траекторию
            if (new_dir != move_dir)
            {
                // Если ориентация траектории изменилась, разворачиваемся
                end_coord = -end_coord;
            }

            if (new_dir == BWD)
            {
                // Если смещаемся назад, начинаем отсчёт с конца траектории
                begin_coord = check_traj->getLength();
                end_coord = end_coord + check_traj->getLength();
            }
            else
            {
                begin_coord = 0.0;
            }
        }
    }

    const size_t initial_vc_count = vehicle_control.size();

    for (size_t i = 0; i < vehicles->size(); ++i)
    {
        VehicleController *vc = new VehicleController;
        Vehicle* const curr_vehicle = (*vehicles)[i];
        const Vehicle* const prev_vehicle = (i == 0) ? nullptr : (*vehicles)[i - 1];

        // Смещаем координату центра данной ПЕ
        // на половину её длины и половину длины предыдущей ПЕ
        double L = curr_vehicle->getLength();
        traj_coord -= static_cast<double>(dir) * L / 2.0;
        if (i != 0)
        {
            traj_coord -= static_cast<double>(dir) * prev_vehicle->getLength() / 2.0;
        }

        if (Trajectory::findTrajectoryAtCoord(cur_traj, traj_coord, dir))
        {
            size_t idx = vehicle_control.size();
            if (curr_vehicle->getModelIndex() != idx)
            {
                Journal::instance()->warning(QString(
                    "Sizes of vehicles array at model and at topology are different."));
                Journal::instance()->warning(QString(
                                                 "For vehicle [%1] index from topology vehicle controller [%2] will be used.")
                                                 .arg(curr_vehicle->getModelIndex())
                                                 .arg(idx));

                curr_vehicle->setModelIndex(idx);
            }
            dir_t veh_dir = static_cast<dir_t>(dir * curr_vehicle->getDirection());
            vc->setIndex(idx);
            vc->setLength(L);
            vc->setVehicleRailwayConnectors(curr_vehicle->getRailwayConnectors());
            vc->setInitCurrentTraj(cur_traj, traj_coord, veh_dir);
            vc->setInitPathCoord(curr_vehicle->getDirection() * curr_vehicle->getTrainCoord());

            vehicle_control.push_back(vc);
            vc_table[curr_vehicle] = vc;

            Journal::instance()->info(QString("Vehcile #%1").arg(idx) +
                                      " at traj: " + cur_traj->getName() +
                                      QString(" %1 m from start").arg(traj_coord));
        }
        else
        {
            // По идее мы уже проверили весь путь по топологии,
            // и не должны попасть сюда, но на всякий случай обработаем
            delete vc;
            Journal::instance()->error(QString("Fail to place Vehicle #%1").arg(vehicle_control.size()) +
                                       " at traj: " + cur_traj->getName() +
                                       QString(" %1 m from start").arg(traj_coord));

            // Очищаем ранее размещённые контроллеры этого поезда
            for (auto it = vehicle_control.begin() + initial_vc_count; it != vehicle_control.end(); ++it)
            {
                for (auto vt = vc_table.begin(); vt != vc_table.end(); ++vt)
                {
                    if (vt->second == *it) { vc_table.erase(vt); break; }
                }
                delete *it;
            }
            vehicle_control.erase(vehicle_control.begin() + initial_vc_count, vehicle_control.end());
            return false;
        }
   }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleController& Topology::getVehicleController(size_t idx)
{
    assert(idx < vehicle_control.size() && "VehicleController index out of range");
    return *vehicle_control[idx];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
route_segment_t Topology::find_route(Trajectory* start_traj,
                                     Trajectory* target_traj,
                                     qint8 dir,
                                     bool check_busy)
{
    // ---------------------------------------------------------------------
    // 0. Входная валидация
    // ---------------------------------------------------------------------
    if (!start_traj || !target_traj)
    {
        Journal::instance()->error("find_route: null trajectory pointer");
        return route_segment_t();
    }

    // ---------------------------------------------------------------------
    // 1. Маршрут на самого себя
    // ---------------------------------------------------------------------
    if (start_traj == target_traj)
    {
        route_segment_t path;
        path.trajectories = { start_traj };
        path.directions   = { (dir < 0) ? BWD : FWD };

        //Journal::instance()->warning("Build route: Target trajectory and start trajectory are the same");

        return path;
    }

    // ---------------------------------------------------------------------
    // 2. Проверка занятости целевой траектории
    // ---------------------------------------------------------------------
    if (target_traj->isBusy() && check_busy)
    {
        Journal::instance()->error("Build route: Target trajectory is busy. Route is impossible");

        return route_segment_t();
    }

    // ---------------------------------------------------------------------
    // 3. Инициализация структур поиска
    // ---------------------------------------------------------------------
    // Очередь: {траектория, направление движения по ней}
    std::queue<std::pair<Trajectory*, dir_t>> queue;

    // Посещённые состояния: ключ = {траектория, направление},
    // значение = {предыдущая траектория, направление от неё}
    // Используем std::map, так как он поддерживает pair как ключ "из коробки"
    std::map<
        std::pair<Trajectory*, dir_t>,
        std::pair<Trajectory*, dir_t>
        > visited;

    const dir_t start_dir = (dir < 0) ? BWD : FWD;

    queue.push({ start_traj, start_dir });
    visited[{ start_traj, start_dir }] = { nullptr, start_dir };

    // ---------------------------------------------------------------------
    // 4. Основной цикл BFS
    // ---------------------------------------------------------------------
    while (!queue.empty())
    {
        const auto [curr_traj, curr_dir] = queue.front();
        queue.pop();

        // -------------------------------------------------------------
        // 4.1. Проверка достижения цели
        // -------------------------------------------------------------
        if (curr_traj == target_traj)
        {
            route_segment_t path;

            // Восстановление пути от цели к старту
            auto path_key = std::make_pair(curr_traj, curr_dir);

            while (visited.count(path_key) && path_key.first != nullptr)
            {
                path.trajectories.push_back(path_key.first);
                path.directions.push_back(path_key.second);
                path_key = visited[path_key];
            }

            // Путь собран в обратном порядке — инвертируем
            std::reverse(path.trajectories.begin(), path.trajectories.end());
            std::reverse(path.directions.begin(), path.directions.end());

            return path;
        }

        // -------------------------------------------------------------
        // 4.2. Получение следующей стрелки
        // -------------------------------------------------------------
        dir_t next_dir = curr_dir;
        Switch* next_switch = curr_traj->getNextSwitch(next_dir);

        if (!next_switch)
        {
            // Тупик — нет продолжения
            continue;
        }

        // -------------------------------------------------------------
        // 4.3. Проверка занятости стрелки
        // -------------------------------------------------------------
        if (check_busy)
        {
            const bool switch_busy =
                (next_switch->getStateFwd() == IN_ROUTE_MINUS) ||
                (next_switch->getStateFwd() == IS_BUSY_MINUS)  ||
                (next_switch->getStateFwd() == IS_BUSY_PLUS)   ||
                (next_switch->getStateFwd() == IN_ROUTE_PLUS)  ||
                (next_switch->getStateBwd() == IN_ROUTE_MINUS) ||
                (next_switch->getStateBwd() == IS_BUSY_MINUS)  ||
                (next_switch->getStateBwd() == IS_BUSY_PLUS)   ||
                (next_switch->getStateBwd() == IN_ROUTE_PLUS);

            if (switch_busy)
            {
                continue;
            }
        }

        // -------------------------------------------------------------
        // 4.4. Перебор всех возможных выходов из стрелки
        // -------------------------------------------------------------
        for (const Switch_way_t& way : switch_ways_t)
        {
            Trajectory* next_traj = next_switch->trajectories[way];

            if (!next_traj)
            {
                continue;
            }

            // Пропускаем, если вернулись на ту же траекторию
            if (next_traj == curr_traj)
            {
                continue;
            }

            // Проверка занятости следующей траектории
            if (check_busy && (next_traj->isBusy() || next_traj->isInRoute()))
            {
                continue;
            }

            // Вычисление направления движения по следующей траектории
            const int orientation = next_switch->getTrajOrientation(next_traj);

            if (orientation == 0)
            {
                // Некорректная ориентация — пропускаем
                continue;
            }

            const dir_t next_traj_dir =
                static_cast<dir_t>(next_dir * orientation);

            // Ключ для проверки посещённости: {траектория, направление}
            const auto visit_key = std::make_pair(next_traj, next_traj_dir);

            if (visited.find(visit_key) == visited.end())
            {
                visited[visit_key] = { curr_traj, curr_dir };
                queue.push({ next_traj, next_traj_dir });
            }
        }
    }

    // ---------------------------------------------------------------------
    // 5. Путь не найден
    // ---------------------------------------------------------------------
    /*Journal::instance()->warning(
        QString("ROUTE NOT FOUND: %1 -> %2. Visited %3 states")
            .arg(start_traj->getName())
            .arg(target_traj->getName())
            .arg(visited.size()));*/

    return route_segment_t();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
route_segment_t Topology::build_route(const route_command_t &rc)
{
    if ((rc.dir != 1) && (rc.dir != -1))
    {
        Journal::instance()->error("BuildRoute: Invalid direction of searching "
                                   + QString::number(rc.dir));
        return route_segment_t();
    }

    Trajectory* s_traj = traj_list.value(rc.trajectory_begin, nullptr);

    if (s_traj == nullptr)
    {
        Journal::instance()->error("BuildRoute: Unknown start trajectory "
                                   + rc.trajectory_begin);
        return route_segment_t();
    }

    Trajectory* t_traj = traj_list.value(rc.trajectory_end, nullptr);

    if (t_traj == nullptr)
    {
        Journal::instance()->error("BuildRoute: Unknown target trajectory "
                                   + rc.trajectory_end);
        return route_segment_t();
    }

    return find_route(s_traj, t_traj, rc.dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::set_switchs_by_route(const route_segment_t& route/*, int dir*/)
{
    for (size_t i = 0; i < route.trajectories.size() - 1; ++i)
    {
        // Берём очередную траекторию маршрута
        Trajectory* prev_traj = route.trajectories[i];

        // Берем у траектории стрелку в направлении построенного маршрута
        dir_t dir = route.directions[i];
        Switch* sw = prev_traj->getNextSwitch(dir);
        if (sw == nullptr)
        {
            Journal::instance()->error(QString("Set switches by route: %1 switch of [%2]%3 is null")
                                           .arg((dir == 1) ? "Fwd" : "Bwd").arg(i).arg(prev_traj->getName()));
            return false;
        }

        // Ожидаемая траектория, исходя из построения маршрута
        Trajectory* next_traj = route.trajectories[i + 1];

        if (dir == FWD)
        {
            // Переключаем попутные остряки
            if (next_traj == sw->trajectories[SW_FWD_PLUS])
            {
                sw->setRefStateFwd(STATE_PLUS);
            }

            if (next_traj == sw->trajectories[SW_FWD_MINUS])
            {
                sw->setRefStateFwd(STATE_MINUS);
            }

            // Переключаем встречные остряки
            if (prev_traj == sw->trajectories[SW_BWD_PLUS])
            {
                sw->setRefStateBwd(STATE_PLUS);
            }

            if (prev_traj == sw->trajectories[SW_BWD_MINUS])
            {
                sw->setRefStateBwd(STATE_MINUS);
            }
        }
        else if (dir == -1)
        {
            // Переключаем попутные остряки
            if (next_traj == sw->trajectories[SW_BWD_PLUS])
            {
                sw->setRefStateBwd(STATE_PLUS);
            }

            if (next_traj == sw->trajectories[SW_BWD_MINUS])
            {
                sw->setRefStateBwd(STATE_MINUS);
            }

            // Переключаем встречные остряки
            if (prev_traj == sw->trajectories[SW_FWD_PLUS])
            {
                sw->setRefStateFwd(STATE_PLUS);
            }

            if (prev_traj == sw->trajectories[SW_FWD_MINUS])
            {
                sw->setRefStateFwd(STATE_MINUS);
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::open_route_signals(const route_segment_t& route,
                                  std::vector<std::pair<QString, int>>& sw_list,
                                  bool for_train)
{
    for (size_t i = 0; i < route.trajectories.size() - 1; ++i)
    {
        // Берём очередную траекторию маршрута
        Trajectory* traj = route.trajectories[i];

        // Берем у траектории стрелку в направлении построенного маршрута
        dir_t dir = route.directions[i];
        Switch* sw = traj->getNextSwitch(dir);
        if (sw == nullptr)
        {
            Journal::instance()->error(QString("Open route signals: %1 switch of [%2]%3 is null")
                                           .arg((dir == 1) ? "Fwd" : "Bwd").arg(i).arg(traj->getName()));
            return false;
        }

        // Проверяем, есть ли на ней сигнал
        Signal* signal = (dir == 1) ? sw->getSignalFwd() : sw->getSignalBwd();

        if (signal == nullptr)
        {
            // нет, и открывать нечего, идем дальше
            continue;
        }

        if (for_train)
        {
            if (StationSignal* station_sig = dynamic_cast<StationSignal *>(signal))
            {
                // Добавляем в список поездной светофор
                sw_list.push_back({sw->getName(), dir});
            }
        }
        else
        {
            // Добавляем в список
            sw_list.push_back({sw->getName(), dir});
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::step(double t, double dt)
{
    for (Trajectory* const traj : traj_list)
    {
        traj->clearBusy();
    }

    for (VehicleController* const vc : vehicle_control)
    {
        vc->step(t, dt);
    }

    for (Trajectory* const traj : traj_list)
    {
        traj->step(t, dt);
    }

    for (Switch* const sw : switches)
    {
        sw->step(t, dt);
    }

    for (const auto* signals_array : {&signals_data.line_signals,
                                      &signals_data.enter_signals,
                                      &signals_data.route_signals,
                                      &signals_data.exit_signals,
                                      &signals_data.shunt_signals})
    {
        for (Signal* const signal : *signals_array)
        {
            if (signal)
            {
                signal->step(t, dt);
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Topology::serialize()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << route_name;

    stream << static_cast<uint32_t>(stations.size());

    for (const auto& station : stations)
    {
        QByteArray sdata = station.serialize();
        stream << sdata;
    }

    // Указываем число траекторий
    stream << static_cast<uint32_t>(traj_list.size());

    // Складываем в буфер сериализованную информацию о траекториях
    for (Trajectory* traj : traj_list)
    {
        stream << traj->serialize();
    }

    // Указываем число коннекторов
    stream << static_cast<uint32_t>(switches.size());

    // Складываем в буфер сериализованную информацию о коннекторах
    for (Switch* sw : switches)
    {
        stream << sw->serialize();
    }

    return data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> route_name;

    uint32_t stations_count = 0;
    stream >> stations_count;

    stations.clear();

    for (uint32_t i = 0; i < stations_count; ++i)
    {
        QByteArray station_data;
        stream >> station_data;
        topology_station_t station;
        station.deserialize(station_data);
        stations.push_back(station);
    }

    traj_list.clear();
    switches.clear();

    // Число траекторий
    uint32_t traj_count = 0;
    stream >> traj_count;

    // Создаём все траектории
    for (uint32_t i = 0; i < traj_count; ++i)
    {
        Trajectory *traj = new Trajectory;;

        QByteArray traj_data;
        stream >> traj_data;
        traj->deserialize(traj_data);

        traj_list.insert(traj->getName(), traj);
    }

    // Число коннекторов
    uint32_t conn_count = 0;
    stream >> conn_count;

    // Создаём все коннекторы
    for (uint32_t i = 0; i < conn_count; ++i)
    {
        Switch *sw = new Switch;

        QByteArray conn_data;
        stream >> conn_data;
        sw->deserialize(conn_data, traj_list);

        switches.insert(sw->getName(), sw);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
traj_list_t *Topology::getTrajectoriesList()
{
    return &traj_list;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const traj_list_t* Topology::getTrajectoriesList() const
{
    return &traj_list;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sw_list_t *Topology::getConnectorsList()
{
    return &switches;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const sw_list_t* Topology::getConnectorsList() const
{
    return &switches;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
topology_stations_list_t *Topology::getStationsList()
{
    return &stations;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
signals_data_t *Topology::getSignalsData()
{
    return &signals_data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Topology::getRouteName() const
{
    return route_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList Topology::getTrajNamesList(QString route_dir)
{
    QString path = route_dir + QDir::separator() +
                   "topology" + QDir::separator() +
                   + "trajectories";

    QDir traj_dir(path);

    Journal::instance()->info("Check trajectories at directory " + path);

    QDirIterator traj_files(traj_dir.path(),
                            QStringList() << "*.traj",
                            QDir::NoDotAndDotDot | QDir::Files);

    QStringList names_list;

    while (traj_files.hasNext())
    {
        QString fullpath = traj_files.next();

        Journal::instance()->info("Found trajectory " + fullpath);

        QFileInfo file_info(fullpath);

        names_list << file_info.baseName();
    }

    return names_list;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<std::vector<module_cfg_t>> Topology::load_topology_configs(QString route_path)
{
    // Загрузка модулей для траекторий
    // Если в /<route>/topology есть папки с названием вида trajectory-*
    // то будут загружены модули с таким же названием (/modules/trajectory-*.dll)
    const QString topology_path = route_path + QDir::separator() + "topology";
    const QDir topology_dir = QDir(topology_path);

    const QStringList traj_modules_dirs = topology_dir.entryList(
        {"trajectory-*"}, QDir::Dirs);

    // Из папок trajectory-* загружаем все конфиги *.xml
    std::vector<std::vector<module_cfg_t>> all_modules;
    for (const QString& name : traj_modules_dirs)
    {
        if (name.isEmpty())
        {
            continue;
        }

        const QString traj_module_path = topology_path +
            QDir::separator() + name;

        const QDir traj_module_dir = QDir(traj_module_path);

        const QStringList cfg_files = traj_module_dir.entryList(
            {"*.xml"}, QDir::Files);

        std::vector<module_cfg_t> all_cfgs;
        for (const QString& cfg_name : cfg_files)
        {
            if (cfg_name.isEmpty())
            {
                continue;
            }

            module_cfg_t module_config;

            const QString cfg_path = traj_module_path +
                QDir::separator() + cfg_name;

            if (!module_config.cfg.load(cfg_path))
            {
                continue;
            }

            module_config.module_name = name;

            // Список траекторий в этом конфиге:
            // модуль будет подгружен к траекториям,
            // имя которой указано хотя бы в одном конфиге,
            // после чего настроен этим же конфигом
            QDomNode trajNode = module_config.cfg.getFirstSection(
                "Trajectory");

            while (!trajNode.isNull())
            {
                QString traj_name;
                module_config.cfg.getString(trajNode, "Name", traj_name);

                if (traj_name.isEmpty())
                {
                    Journal::instance()->warning("Empty trajectory name at " + cfg_path);
                }
                else
                {
                    module_config.traj_names.insert(traj_name);
                }

                trajNode = module_config.cfg.getNextSection();
            }

            if (module_config.traj_names.empty())
            {
                Journal::instance()->warning("No trajectories found in " + cfg_path);
            }
            else
            {
                all_cfgs.push_back(module_config);
            }
        }

        if (all_cfgs.empty())
        {
            Journal::instance()->warning("No trajectories found in files at " + traj_module_path);
        }
        else
        {
            all_modules.push_back(all_cfgs);
        }
    }

    return all_modules;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::load_topology(QString route_dir)
{
    QString path = route_dir + QDir::separator() +
                   "topology" + QDir::separator() +
                   "topology.xml";

    CfgReader cfg;

    if (!cfg.load(path))
    {
        Journal::instance()->error("File " + path + " not found");
        return false;
    }

    QDomNode secNode = cfg.getFirstSection("Switch");

    while (!secNode.isNull())
    {
        Switch *sw = new Switch();
        sw->configure(cfg, secNode, traj_list);

        switches.insert(sw->getName(), sw);
        connect(sw, &Switch::sendSwitchState, this, &Topology::sendSwitchState);

        load_signals(cfg, secNode, sw);

        secNode = cfg.getNextSection();
    }
/*
    secNode = cfg.getFirstSection("Joint");

    while (!secNode.isNull())
    {
        IsolatedJoint *joint = new IsolatedJoint();
        joint->configure(cfg, secNode, traj_list);

        joints.insert(joint->getName(), joint);

        secNode = cfg.getNextSection();
    }
*/
    // Увяжем сигналы траектории со слотами топологии
    for (auto traj : traj_list)
    {
        connect(traj, &Trajectory::sigTrajChangeState, this, &Topology::slotTrajChangeState);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool strignToVector(const QString &str, dvec3 &vector)
{
    QStringList tokens = str.split(' ');

    if (tokens.size() < 3)
    {
        return false;
    }

    vector.x = tokens[0].toDouble();
    vector.y = tokens[1].toDouble();
    vector.z = tokens[2].toDouble();

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::load_signals(CfgReader& cfg, QDomNode secNode, Switch* sw)
{
    auto configure_signal = [](Signal* signal, Switch* sw, dir_t direction,
                               QString signal_letter, QString signal_model,
                               dvec3 relative_position, dvec3 relative_rotation)
    {
        if (direction == FWD)
        {
            sw->setSignalFwd(signal);
        }
        else if (direction == BWD)
        {
            sw->setSignalBwd(signal);
        }

        signal->setConnector(sw);
        signal->setDirection(direction);
        signal->setLetter(signal_letter);
        signal->setSignalModel(signal_model);
        signal->setRelPosition(relative_position);
        signal->setRelRotation(relative_rotation);
    };

    QString signal_model_fwd = "";
    QString signal_model_bwd = "";

    if (cfg.getString(secNode, "SignalModelFwd", signal_model_fwd))
    {
        QString tmp;
        dvec3 rel_pos = {0.0, 0.0, 0.0};
        dvec3 rel_rot = {0.0, 0.0, 0.0};

        tmp = "";
        cfg.getString(secNode, "RelPosVectorFwd", tmp);

        if (!tmp.isEmpty())
            strignToVector(tmp, rel_pos);

        tmp = "";
        cfg.getString(secNode, "RelRotVectorFwd", tmp);

        if (!tmp.isEmpty())
            strignToVector(tmp, rel_rot);

        QString signal_letter = "";
        cfg.getString(secNode, "SignalLiterFwd", signal_letter);

        if (signal_model_fwd.right(4) == "line")
        {
            LineSignal *signal = new LineSignal;
            configure_signal(signal, sw, FWD,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.line_signals.push_back(signal);
            Journal::instance()->info("Loaded line signal " + signal->getLetter());
        }
        else if (signal_model_fwd.right(4) == "entr")
        {
            EnterSignal *signal = new EnterSignal;
            configure_signal(signal, sw, FWD,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.enter_signals.push_back(signal);
            Journal::instance()->info("Loaded enter signal " + signal->getLetter());
        }
        else if (signal_model_fwd.right(4) == "rout")
        {
            RouteSignal *signal = new RouteSignal;
            configure_signal(signal, sw, FWD,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.route_signals.push_back(signal);
            Journal::instance()->info("Loaded route signal " + signal->getLetter());
        }
        else if (signal_model_fwd.right(4) == "exit")
        {
            ExitSignal *signal = new ExitSignal;
            configure_signal(signal, sw, FWD,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.exit_signals.push_back(signal);
            Journal::instance()->info("Loaded exit signal " + signal->getLetter());
        }
        else if (signal_model_fwd.right(4) == "shnt")
        {
            ShuntingSignal *signal = new ShuntingSignal;
            configure_signal(signal, sw, FWD,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.shunt_signals.push_back(signal);
            Journal::instance()->info("Loaded shunting signal " + signal->getLetter());
        }
    }

    if (cfg.getString(secNode, "SignalModelBwd", signal_model_bwd))
    {
        QString tmp;
        dvec3 rel_pos = {0.0, 0.0, 0.0};
        dvec3 rel_rot = {0.0, 0.0, 0.0};

        tmp = "";
        cfg.getString(secNode, "RelPosVectorBwd", tmp);

        if (!tmp.isEmpty())
            strignToVector(tmp, rel_pos);

        tmp = "";
        cfg.getString(secNode, "RelRotVectorBwd", tmp);

        if (!tmp.isEmpty())
            strignToVector(tmp, rel_rot);

        QString signal_letter = "";
        cfg.getString(secNode, "SignalLiterBwd", signal_letter);

        if (signal_model_bwd.right(4) == "line")
        {
            LineSignal *signal = new LineSignal;
            configure_signal(signal, sw, BWD,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.line_signals.push_back(signal);
            Journal::instance()->info("Loaded line signal " + signal->getLetter());
        }
        else if (signal_model_bwd.right(4) == "entr")
        {
            EnterSignal *signal = new EnterSignal;
            configure_signal(signal, sw, BWD,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.enter_signals.push_back(signal);
            Journal::instance()->info("Loaded enter signal " + signal->getLetter());
        }
        else if (signal_model_bwd.right(4) == "rout")
        {
            RouteSignal *signal = new RouteSignal;
            configure_signal(signal, sw, BWD,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.route_signals.push_back(signal);
            Journal::instance()->info("Loaded route signal " + signal->getLetter());
        }
        else if (signal_model_bwd.right(4) == "exit")
        {
            ExitSignal *signal = new ExitSignal;
            configure_signal(signal, sw, BWD,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.exit_signals.push_back(signal);
            Journal::instance()->info("Loaded exit signal " + signal->getLetter());
        }
        else if (signal_model_bwd.right(4) == "shnt")
        {
            ShuntingSignal *signal = new ShuntingSignal;
            configure_signal(signal, sw, BWD,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.shunt_signals.push_back(signal);
            Journal::instance()->info("Loaded shunting signal " + signal->getLetter());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::load_stations(QString route_dir)
{
    QString path = route_dir + QDir::separator() +
                   "topology" + QDir::separator() +
                   "stations.conf";

    QFile stations_file(path);

    if (!stations_file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QTextStream stream(&stations_file);

    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        QStringList tokens = line.split('\t');

        topology_station_t station;
        station.name = tokens[0];
        station.pos_x = tokens[1].toDouble();
        station.pos_y = tokens[2].toDouble();
        station.pos_z = tokens[3].toDouble();

        stations.push_back(station);

        Journal::instance()->info(QString("Loaded station %1 at {%2,%3,%4}")
                                      .arg(station.name)
                                      .arg(station.pos_x)
                                      .arg(station.pos_y)
                                      .arg(station.pos_z));
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::get_route_name(QString route_dir)
{
    QString path = route_dir + QDir::separator() +
                   "description.xml";

    CfgReader cfg;

    if (!cfg.load(path))
    {
        return;
    }

    QString secName = "Route";
    cfg.getString(secName, "Title", route_name);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::serialize_connector_name(QDataStream& stream, Switch* sw)
{
    if (bool has_sw = sw != nullptr)
    {
        stream << has_sw;
        stream << sw->getName();
    }
    else
    {
        stream << has_sw;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switch* Topology::deserialize_traj_connectors(QDataStream &stream, sw_list_t& sw_list) const
{
    bool has_sw = false;
    stream >> has_sw;

    if (has_sw)
    {
        QString sw_name = "";
        stream >> sw_name;

        return sw_list.value(sw_name, nullptr);
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotGetSwitchState(QByteArray &switch_data)
{
    // Получаем из менеджера Lua данные с запросом на состояние стрелки
    switch_state_t sw_state;
    sw_state.deserialize(switch_data);

    // Ищем стрелку по имени из запроса
    Switch *sw = dynamic_cast<Switch *>(switches.value(sw_state.name, nullptr));

    if (sw == nullptr)
    {
        return;
    }

    // Заполняем в запрос состояние стрелки
    sw_state.state_fwd = sw->getStateFwd();
    sw_state.state_bwd = sw->getStateBwd();

    // Сохраняем состояние в данные из запроса
    switch_data = sw_state.serialize();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotSwitchCommand(QByteArray& switch_command)
{
    switch_command_t sc;
    sc.deserialize(switch_command);

    if (sc.conn_name.isEmpty() || (sc.switch_direction == 0))
    {
        return;
    }

    Switch *sw = dynamic_cast<Switch *>(switches.value(sc.conn_name, nullptr));
    if (sw == nullptr)
    {
        return;
    }

    if (sc.switch_direction < 0)
    {
        sw->setRefStateBwd(static_cast<Switch_state_t>(sc.switch_ref_state));
    }
    else
    {
        sw->setRefStateFwd(static_cast<Switch_state_t>(sc.switch_ref_state));
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotSignalCommand(QByteArray& signal_data)
{
    signal_command_t sc;
    sc.deserialize(signal_data);

    if (sc.conn_name.isEmpty() || (sc.sig_dir == 0))
    {
        return;
    }

    Switch* sw = switches.value(sc.conn_name, nullptr);
    if (sw == nullptr)
    {
        return;
    }

    Signal* sig = (sc.sig_dir < 1) ? sw->getSignalBwd() : sw->getSignalFwd();
    if (sig == nullptr)
    {
        return;
    }

    // Маршрутный сигнал сделан из входного, сперва проверяем каст к нему
    if (RouteSignal* rs = dynamic_cast<RouteSignal *>(sig))
    {
        if (sc.command_open_train)
        {
            rs->slotPressOpenTrain();
            return;
        }
        if (sc.command_open_shunting)
        {
            rs->slotPressOpenShunting();
            return;
        }
        if (sc.command_open_call)
        {
            rs->slotPressOpenCall();
            return;
        }
        if (sc.command_close)
        {
            rs->slotPressClose();
            return;
        }
        return;
    }

    if (EnterSignal* es = dynamic_cast<EnterSignal *>(sig))
    {
        if (sc.command_open_train)
        {
            es->slotPressOpenTrain();
            return;
        }
        if (sc.command_open_call)
        {
            es->slotPressOpenCall();
            return;
        }
        if (sc.command_close)
        {
            es->slotPressClose();
            return;
        }
        return;
    }

    if (ExitSignal* es = dynamic_cast<ExitSignal *>(sig))
    {
        if (sc.command_open_train)
        {
            es->slotPressOpenTrain();
            return;
        }
        if (sc.command_open_shunting)
        {
            es->slotPressOpenShunting();
            return;
        }
        if (sc.command_open_call)
        {
            es->slotPressOpenCall();
            return;
        }
        if (sc.command_close)
        {
            es->slotPressClose();
            return;
        }
        return;
    }

    if (ShuntingSignal* ss = dynamic_cast<ShuntingSignal *>(sig))
    {
        if (sc.command_open_shunting)
        {
            ss->slotPressOpenShunting();
            return;
        }
        if (sc.command_close)
        {
            ss->slotPressClose();
            return;
        }
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotBuildRouteCommand(QByteArray &route_data)
{
    route_command_t rc;
    rc.deserialize(route_data);

    route_segment_t route = build_route(rc);

    if (route.trajectories.empty())
    {
        Journal::instance()->error("Build route: No route from "
                                   + rc.trajectory_begin + " to " + rc.trajectory_end);
        return;
    }
    Journal::instance()->info("Build route: founded from "
                              + rc.trajectory_begin + " to " + rc.trajectory_end
                              + " through " + QString::number(route.trajectories.size()) + " trajectories");

    set_switchs_by_route(route);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotTrainRouteCommand(QByteArray &route_data)
{
    route_command_t rc;
    rc.deserialize(route_data);

    route_segment_t route = build_route(rc);

    if (route.trajectories.empty())
    {
        Journal::instance()->error("Build route: No route from "
                                   + rc.trajectory_begin + " to " + rc.trajectory_end);
        return;
    }
    Journal::instance()->info("Build route: founded from "
                              + rc.trajectory_begin + " to " + rc.trajectory_end
                              + " through " + QString::number(route.trajectories.size()) + " trajectories");

    if (set_switchs_by_route(route))
    {
        std::vector<std::pair<QString, int>> signals_for_open;
        open_route_signals(route, signals_for_open, true);

        emit sigSetOpenSignalsQueue(signals_for_open, true, false);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotShuntingRouteCommand(QByteArray &route_data)
{
    route_command_t rc;
    rc.deserialize(route_data);

    route_segment_t route = build_route(rc);

    if (route.trajectories.empty())
    {
        Journal::instance()->error("Build route: No route from "
                                   + rc.trajectory_begin + " to " + rc.trajectory_end);
        return;
    }
    Journal::instance()->info("Build route: founded from "
                              + rc.trajectory_begin + " to " + rc.trajectory_end
                              + " through " + QString::number(route.trajectories.size()) + " trajectories");

    if (set_switchs_by_route(route))
    {
        std::vector<std::pair<QString, int>> signals_for_open;
        open_route_signals(route, signals_for_open, false);

        emit sigSetOpenSignalsQueue(signals_for_open, false, true);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotGetTrajState(QString traj_name, bool &is_busy, bool &in_route)
{
    if (traj_name.isEmpty())
    {
        return;
    }

    auto* traj = traj_list.value(traj_name, nullptr);

    if (traj == nullptr)
    {
        return;
    }

    is_busy = traj->isBusy();
    in_route = traj->isInRoute();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotGetNextTrajName(QString traj_name, int dir, QString &next_traj_name)
{
    if (traj_name.isEmpty())
    {
        return;
    }

    auto traj = traj_list.value(traj_name, nullptr);

    if (traj == nullptr)
    {
        return;
    }

    dir_t direction = (dir < 0) ? BWD : FWD;
    Switch* sw = traj->getNextSwitch(direction);

    if (sw == nullptr)
    {
        return;
    }

    Trajectory *next_traj = sw->getNextTraj(direction);

    if (next_traj == nullptr)
    {
        return;
    }

    next_traj_name = next_traj->getName();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotIsRouteExists(QString start_traj_name,
                                 QString end_traj_name,
                                 int dir,
                                 bool *exists)
{
    auto start_traj = traj_list.value(start_traj_name, nullptr);

    if (start_traj == nullptr)
    {
        *exists = false;
        return;
    }

    auto end_traj = traj_list.value(end_traj_name, nullptr);

    if (end_traj == nullptr)
    {
        *exists = false;
        return;
    }

    auto route_seg = find_route(start_traj, end_traj, dir, false);

    if (route_seg.trajectories.empty())
    {
        *exists = false;
    }
    else
    {
        *exists = true;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotGetRouteLength(int vehicle_idx, QString cur_traj_name, double cur_coord,
                                  QString target_traj_name, double target_coord,
                                  int dir, double *lenght)
{
    auto cur_traj = traj_list.value(cur_traj_name, nullptr);

    if (cur_traj == nullptr)
    {
        *lenght = -1;
        return;
    }

    auto target_traj = traj_list.value(target_traj_name, nullptr);

    if (target_traj == nullptr)
    {
        *lenght = -1;
        return;
    }

    auto route_seg = find_route(cur_traj, target_traj, dir, false);

    if (route_seg.trajectories.empty())
    {
        *lenght = -1;
        return;
    }

    // Траектория одна
    if (route_seg.trajectories.size() == 1)
    {
        *lenght = (target_coord - cur_coord) * route_seg.directions[0];

        if (*lenght < 0)
        {
            double eps = 1.0;
            double piket_length = 100.0;
            bool is_on_target_traj = (cur_coord >= eps) && (cur_coord <= target_traj->getLength() - eps) && (*lenght >= -piket_length);

            emit sigIncTargetStation(vehicle_idx, is_on_target_traj);
        }
        else
        {
            emit sigCalcMiddleVelocity(vehicle_idx, *lenght);
        }

        return;
    }

    *lenght = 0.0;

    if (route_seg.directions.front() == FWD)
    {
        *lenght += route_seg.trajectories.front()->getLength() - cur_coord;
    }
    else
    {
        *lenght += cur_coord;
    }

    if (route_seg.directions.back() == FWD)
    {
        *lenght += target_coord;
    }
    else
    {
        *lenght += route_seg.trajectories.back()->getLength() - target_coord;
    }

    if (route_seg.trajectories.size() == 2)
    {
        emit sigCalcMiddleVelocity(vehicle_idx, *lenght);
        return;
    }

    // Траекторий 3 и более
    // Серединка из полных траекторий
    for (size_t i = 1; i < route_seg.trajectories.size() - 1; ++i)
    {
        *lenght += route_seg.trajectories[i]->getLength();
    }

    emit sigCalcMiddleVelocity(vehicle_idx, *lenght);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotGetTrajStateRequest(int vehicle_idx, int station_idx, QString start_traj_name, QString traj_name, int dir, int request_type)
{
    QString request_name = "";

    request_type == 0 ? request_name = "ARRIVAL" : request_name = "DEPARTURE";

    QString msg = QString("TIMETABLE PROCESS: vehicle #%1 request from %2 to %3 build %4 route")
                      .arg(vehicle_idx)
                      .arg(start_traj_name)
                      .arg(traj_name)
                      .arg(request_name);

    Journal::instance()->debug(msg);

    // Проверяем статус траектории
    bool is_busy = false;
    bool in_route = false;

    slotGetTrajState(traj_name, is_busy, in_route);

    if (is_busy || in_route)
    {
        if (is_busy)
        {
            Journal::instance()->debug(QString("TIMETABLE PROCESS: vehicle #%1 target trajectory %2 is busy").arg(vehicle_idx).arg(traj_name));
        }

        if (in_route)
        {
            Journal::instance()->debug(QString("TIMETABLE PROCESS: vehicle #%1 target trajectory %2 is in other route").arg(vehicle_idx).arg(traj_name));
        }

        emit sigGetTrajState(vehicle_idx, station_idx, start_traj_name, traj_name, request_type, false);
        return;
    }

    // Проверяем, возможен ли данный маршрут
    auto* start_traj = traj_list.value(start_traj_name, nullptr);

    if (start_traj == nullptr)
    {
        Journal::instance()->debug(QString("TIMETABLE PROCESS: vehicle #%1 start trajectory %2 not exists").arg(vehicle_idx).arg(start_traj_name));
        emit sigGetTrajState(vehicle_idx, station_idx, start_traj_name, traj_name, request_type, false);
        return;
    }

    auto* traj = traj_list.value(traj_name, nullptr);

    if (traj == nullptr)
    {
        Journal::instance()->debug(QString("TIMETABLE PROCESS: vehicle #%1 target trajectory %2 not exists").arg(vehicle_idx).arg(traj_name));
        emit sigGetTrajState(vehicle_idx, station_idx, start_traj_name, traj_name, request_type, false);
        return;
    }

    auto route_seg = find_route(start_traj, traj, dir);

    if (route_seg.trajectories.empty())
    {
        Journal::instance()->debug(QString("TIMETABLE PROCESS: route is not available for vehicle #%1").arg(vehicle_idx));
    }

    emit sigGetTrajState(vehicle_idx, station_idx, start_traj_name, traj_name, request_type, !route_seg.trajectories.empty());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotTrajChangeState(int vehicle_idx, bool is_busy, QString traj_name)
{
    if (vehicle_idx < 0 || static_cast<size_t>(vehicle_idx) >= vehicle_control.size())
        return;

    // Определяем поезд, изменивший состояние траектории
    size_t train_idx = vehicle_control[vehicle_idx]->getTrainIndex();

    emit sigChangeTrajStateByTrain(static_cast<int>(train_idx), is_busy, traj_name);
}
