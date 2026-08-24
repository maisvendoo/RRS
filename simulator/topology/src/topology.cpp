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
#include    <algorithm>
#include    <cmath>
#include    <QDir>
#include    <QDirIterator>
#include    <QFile>

#include    <CfgReader.h>
#include    <switch.h>
#include    <switch-state.h>
#include    <line-signal.h>

#include    <Journal.h>
#include    <filesystem.h>

#include    "speed-limit-source.h"
#include    <topology-trajectory-device.h>

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
        connect(traj, &Trajectory::sendModuleUpdate, this, &Topology::sendModuleUpdate);
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
    for (const Vehicle* vehicle : *vehicles)
    {
        train_length += vehicle->getLength();
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
QByteArray Topology::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << route_name;

    stream << static_cast<uint32_t>(stations.size());

    for (const auto& station : stations)
    {
        stream << station.serialize();
    }

    // Указываем число траекторий
    stream << static_cast<uint32_t>(traj_list.size());

    // Складываем в буфер сериализованную информацию о траекториях
    for (const Trajectory* traj : traj_list)
    {
        stream << traj->serialize();
    }

    // Указываем число коннекторов
    stream << static_cast<uint32_t>(switches.size());

    // Складываем в буфер сериализованную информацию о коннекторах
    for (const Switch* sw : switches)
    {
        stream << sw->serialize();
    }

    return data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Topology::serialize_modules() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // Указываем число траекторий
    stream << static_cast<uint32_t>(traj_list.size());

    // Складываем в буфер сериализованную информацию о путевой инфраструктуре траекторий
    for (const Trajectory* traj : traj_list)
    {
        stream << traj->getName();
        stream << traj->serialize_modules();
    }

    return data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Topology::serialize_stations() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    // Указываем число станций
    stream << static_cast<uint32_t>(stations.size());

    // Складываем в буфер сериализованную информацию о станциях
    for (const auto& station : stations)
    {
        stream << station.serialize();
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
void Topology::deserialize_modules(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    // Число траекторий
    uint32_t traj_count = 0;
    stream >> traj_count;

    // Добавляем модули в траектории
    for (uint32_t i = 0; i < traj_count; ++i)
    {
        QString traj_name;
        stream >> traj_name;

        QByteArray modules_data;
        stream >> modules_data;

        if (traj_name.isEmpty())
        {
            continue;
        }

        Trajectory* traj = traj_list.value(traj_name, nullptr);

        if (traj)
        {
            traj->deserialize_modules(modules_data);
        }
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
                all_cfgs.emplace_back(std::move(module_config));
            }
        }

        if (all_cfgs.empty())
        {
            Journal::instance()->warning("No trajectories found in files at " + traj_module_path);
        }
        else
        {
            all_modules.emplace_back(std::move(all_cfgs));
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
void Topology::serialize_connector_name(QDataStream& stream, const Switch* sw) const
{
    bool has_sw = (sw != nullptr);
    stream << has_sw;
    if (has_sw)
    {
        stream << sw->getName();
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
    // Проверяем статус траектории
    bool is_busy = false;
    bool in_route = false;

    slotGetTrajState(traj_name, is_busy, in_route);

    if (is_busy || in_route)
    {
        if (is_busy)
        {
            Journal::instance()->debug(QString("ROUTE REQUEST for vehicle #%1: target trajectory %2 is busy")
                                           .arg(vehicle_idx).arg(traj_name));
        }

        if (in_route)
        {
            Journal::instance()->debug(QString("ROUTE REQUEST for vehicle #%1: target trajectory %2 is in other route")
                                           .arg(vehicle_idx).arg(traj_name));
        }

        emit sigGetTrajState(vehicle_idx, station_idx, start_traj_name, traj_name, request_type, false);
        return;
    }

    // Проверяем, возможен ли данный маршрут
    auto* start_traj = traj_list.value(start_traj_name, nullptr);

    if (start_traj == nullptr)
    {
        Journal::instance()->debug(QString("ROUTE REQUEST for vehicle #%1: start trajectory %2 not exists")
                                       .arg(vehicle_idx).arg(start_traj_name));
        emit sigGetTrajState(vehicle_idx, station_idx, start_traj_name, traj_name, request_type, false);
        return;
    }

    auto* traj = traj_list.value(traj_name, nullptr);

    if (traj == nullptr)
    {
        Journal::instance()->debug(QString("ROUTE REQUEST for vehicle #%1: target trajectory %2 not exists")
                                       .arg(vehicle_idx).arg(traj_name));
        emit sigGetTrajState(vehicle_idx, station_idx, start_traj_name, traj_name, request_type, false);
        return;
    }

    auto route_seg = find_route(start_traj, traj, dir);

    if (route_seg.trajectories.empty())
    {
        Journal::instance()->debug(QString("ROUTE REQUEST for vehicle #%1: not available route from %2 to %3")
                                       .arg(vehicle_idx).arg(start_traj_name).arg(traj_name));
    }

    emit sigGetTrajState(vehicle_idx, station_idx, start_traj_name, traj_name, request_type, !route_seg.trajectories.empty());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotTrajModuleUpdate(QByteArray& traj_module_data)
{
    QDataStream stream(&traj_module_data, QIODevice::ReadOnly);

    QString traj_name;
    stream >> traj_name;
    std::uint32_t module_idx;
    stream >> module_idx;
    QByteArray modules_data;
    stream >> modules_data;

    if (traj_name.isEmpty())
    {
        return;
    }

    Trajectory* traj = traj_list.value(traj_name, nullptr);
    if (!traj)
    {
        return;
    }

    traj->deserializeModuleUpdate(module_idx, modules_data);
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
namespace
{
    //------------------------------------------------------------------------------
    // Сегмент профиля с именем траектории
    //------------------------------------------------------------------------------
    struct traj_segment_t
    {
        double begin;
        double end;
        QString name;
    };

    //------------------------------------------------------------------------------
    // Индекс участка (track_t), содержащего координату
    //------------------------------------------------------------------------------
    int findTrackIndex(const std::vector<track_t>& tracks, double coord)
    {
        if (tracks.empty())
            return 0;

        int lo = 0;
        int hi = static_cast<int>(tracks.size()) - 1;
        while (lo < hi)
        {
            int mid = (lo + hi) / 2;
            if (coord < tracks[mid].traj_coord)
                hi = mid;
            else if (coord >= tracks[mid].traj_coord + tracks[mid].len)
                lo = mid + 1;
            else
                return mid;
        }
        return lo;
    }

    //------------------------------------------------------------------------------
    // Точка профиля в заданной координате траектории
    //------------------------------------------------------------------------------
    profile_segment_t pointAtCoord(const Trajectory* traj, double coord, dir_t orient, double distance)
    {
        profile_segment_t point;

        profile_point_t pp = traj->getPosition(coord, orient);

        point.distance = distance;
        point.elevation = pp.position.z;
        point.railway_coord = pp.railway_coord;

        return point;
    }

    //------------------------------------------------------------------------------
    // Точки профиля на границах участков (track_t) траектории
    //------------------------------------------------------------------------------
    void emitTrackBoundaries(const Trajectory* traj, double coord, double stop_coord, dir_t orient,
                             double traveled, int kind, std::vector<profile_segment_t>& out)
    {
        const std::vector<track_t>& tracks = traj->getTracks();
        if (tracks.empty())
            return;

        const double eps = 1e-9;

        if (orient == FWD)
        {
            double cursor = coord;
            int ti = findTrackIndex(tracks, cursor);
            while (ti < static_cast<int>(tracks.size()))
            {
                const track_t& track = tracks[ti];
                double far = track.traj_coord + track.len;
                if (far - cursor < eps)
                {
                    ++ti;
                    continue;
                }
                if (far >= stop_coord - eps)
                    break;
                out.push_back(pointAtCoord(traj, far, orient, kind * (traveled + (far - coord))));
                cursor = far;
                ++ti;
            }
            out.push_back(pointAtCoord(traj, stop_coord, orient, kind * (traveled + (stop_coord - coord))));
        }
        else
        {
            double cursor = coord;
            int ti = findTrackIndex(tracks, cursor);
            while (ti >= 0)
            {
                const track_t& track = tracks[ti];
                double near = track.traj_coord;
                if (cursor - near < eps)
                {
                    --ti;
                    continue;
                }
                if (near <= stop_coord + eps)
                    break;
                out.push_back(pointAtCoord(traj, near, orient, kind * (traveled + (coord - near))));
                cursor = near;
                --ti;
            }
            out.push_back(pointAtCoord(traj, stop_coord, orient, kind * (traveled + (coord - stop_coord))));
        }
    }

    //------------------------------------------------------------------------------
    // Единицы подвижного состава, занимающие пройденный участок траектории
    //------------------------------------------------------------------------------
    void collectVehicles(const Trajectory* traj, double entry_coord, double stop_coord, dir_t orient,
                         double traveled, int kind, std::vector<profile_vehicle_t>& out)
    {
        const QMap<size_t, std::array<double, 2>>& busy = traj->getVehiclesCoords();
        if (busy.isEmpty())
            return;

        const double w_lo = std::min(entry_coord, stop_coord);
        const double w_hi = std::max(entry_coord, stop_coord);

        for (auto it = busy.constBegin(); it != busy.constEnd(); ++it)
        {
            const double vb = it.value()[0];
            const double ve = it.value()[1];

            // Пересечение интервала ПЕ с пройденным диапазоном координат
            const double ov_b = std::max(vb, w_lo);
            const double ov_e = std::min(ve, w_hi);
            if (ov_e <= ov_b)
                continue;

            double d0, d1;
            if (orient == FWD)
            {
                d0 = kind * (traveled + (ov_b - entry_coord));
                d1 = kind * (traveled + (ov_e - entry_coord));
            }
            else
            {
                d0 = kind * (traveled + (entry_coord - ov_b));
                d1 = kind * (traveled + (entry_coord - ov_e));
            }

            profile_vehicle_t v;
            v.vehicle_id = it.key();
            v.begin_distance = std::min(d0, d1);
            v.end_distance = std::max(d0, d1);
            out.push_back(v);
        }
    }

    //------------------------------------------------------------------------------
    // Станции на пройденном участке траектории: для каждого трека ищем станцию,
    // проекция которой на ось трека попадает в пределы трека, а удаление станции
    // от оси пути не превышает заданного порога
    //------------------------------------------------------------------------------
    void collectStations(const Trajectory* traj, double entry_coord, double stop_coord, dir_t orient,
                         double traveled, int kind,
                         const topology_stations_list_t& stations,
                         std::vector<profile_station_t>& out)
    {
        if (stations.isEmpty())
            return;

        const std::vector<track_t>& tracks = traj->getTracks();
        if (tracks.empty())
            return;

        const double eps = 1e-9;
        // Порог захвата станций: расстояние от оси пути поезда. Берём с запасом,
        // т.к. на многопутных участках станция может стоять у соседнего пути
        // (например, блок-пост между путями главного хода на расстоянии ~50 м)
        const double max_station_dist = 100.0;

        const double w_lo = std::min(entry_coord, stop_coord);
        const double w_hi = std::max(entry_coord, stop_coord);

        for (const track_t& track : tracks)
        {
            const double t_lo = track.traj_coord;
            const double t_hi = track.traj_coord + track.len;

            // Трек не пересекает пройденный диапазон
            if ((t_hi <= w_lo + eps) || (t_lo >= w_hi - eps))
                continue;

            for (const topology_station_t& st : stations)
            {
                // Проекция станции на ось трека в плане (высоту не учитываем)
                const double dx = st.pos_x - track.begin_point.x;
                const double dy = st.pos_y - track.begin_point.y;
                const double proj = dx * track.orth.x + dy * track.orth.y;

                if ((proj < -eps) || (proj > track.len + eps))
                    continue;

                // Перпендикулярное удаление станции от оси пути в плане
                const double perp_x = dx - proj * track.orth.x;
                const double perp_y = dy - proj * track.orth.y;
                if (std::sqrt(perp_x * perp_x + perp_y * perp_y) > max_station_dist)
                    continue;

                const double st_coord = track.traj_coord + proj;

                profile_station_t ps;
                ps.distance = (orient == FWD)
                    ? kind * (traveled + (st_coord - entry_coord))
                    : kind * (traveled + (entry_coord - st_coord));
                ps.name = st.name;
                out.push_back(ps);
            }
        }
    }

    //------------------------------------------------------------------------------
    // Ограничения скорости на пройденном участке траектории
    //------------------------------------------------------------------------------
    void collectSpeedLimits(const Trajectory* traj, double entry_coord, double stop_coord, dir_t orient,
                           double traveled, int kind, std::vector<profile_speed_limit_t>& out)
    {
        const std::vector<TrajectoryDevice*>& devices = traj->getTrajectoryDevices();
        if (devices.empty())
            return;

        const double w_lo = std::min(entry_coord, stop_coord);
        const double w_hi = std::max(entry_coord, stop_coord);

        for (TrajectoryDevice* dev : devices)
        {
            SpeedLimitSource* sls = dynamic_cast<SpeedLimitSource*>(dev);
            if (sls == nullptr)
                continue;

            std::vector<speed_limit_interval_t> intervals = sls->getSpeedLimits();
            if (intervals.empty())
                continue;

            for (const speed_limit_interval_t& sl : intervals)
            {
                // Интервал ограничения пересекается с пройденным диапазоном
                const double ov_b = std::max(sl.begin, w_lo);
                const double ov_e = std::min(sl.end, w_hi);
                if (ov_e <= ov_b)
                    continue;

                // Метка на entry в зону
                profile_speed_limit_t ps;
                ps.distance = (orient == FWD)
                    ? kind * (traveled + (ov_b - entry_coord))
                    : kind * (traveled + (entry_coord - ov_b));
                ps.end_distance = (orient == FWD)
                    ? kind * (traveled + (ov_e - entry_coord))
                    : kind * (traveled + (entry_coord - ov_e));
                ps.speed_kmh = sl.speed_kmh;
                out.push_back(ps);
            }
        }
    }

    //------------------------------------------------------------------------------
    // Обход топологии от точки отсчёта на заданную дистанцию
    //------------------------------------------------------------------------------
    void walkProfile(Trajectory*& traj, double& coord, dir_t& orient,
                     double limit_m, int kind, std::vector<profile_segment_t>& out,
                     std::vector<traj_segment_t>* segments,
                     std::vector<profile_vehicle_t>* vehicles,
                     std::vector<profile_signal_t>* signal_list,
                     const topology_stations_list_t* stations,
                     std::vector<profile_station_t>* station_list,
                     std::vector<profile_speed_limit_t>* speed_limits)
    {
        if (traj == nullptr || limit_m <= 0.0)
            return;

        const double eps = 1e-9;
        double traveled = 0.0;
        double seg_begin = 0.0;

        auto close_segment = [segments, &traveled, &traj, kind, &seg_begin]()
        {
            if (segments == nullptr)
                return;
            double end = kind * traveled;
            if (std::abs(end - seg_begin) > 1e-9)
                segments->push_back({seg_begin, end, traj->getName()});
        };

        int guard = 0;
        const int max_iters = 100000;

        while ((traveled < limit_m - eps) && (guard < max_iters))
        {
            ++guard;

            double exit_coord = 0.0;
            double within_traj = 0.0;
            if (orient == FWD)
            {
                exit_coord = traj->getLength();
                within_traj = exit_coord - coord;
            }
            else
            {
                exit_coord = 0.0;
                within_traj = coord - exit_coord;
            }

            if (within_traj > eps)
            {
                double step = std::min(limit_m - traveled, within_traj);
                double stop_coord = (orient == FWD) ? (coord + step) : (coord - step);
                emitTrackBoundaries(traj, coord, stop_coord, orient, traveled, kind, out);
                if (vehicles != nullptr)
                    collectVehicles(traj, coord, stop_coord, orient, traveled, kind, *vehicles);
                if (station_list != nullptr && stations != nullptr)
                    collectStations(traj, coord, stop_coord, orient, traveled, kind, *stations, *station_list);
                if (speed_limits != nullptr)
                    collectSpeedLimits(traj, coord, stop_coord, orient, traveled, kind, *speed_limits);
                traveled += step;
                if (traveled >= limit_m - eps)
                    break;
                coord = stop_coord;
            }

            // Переход через стрелку на следующую траекторию
            dir_t exit_dir = orient;
            const Switch* next_sw = traj->getNextSwitch(exit_dir);
            if (next_sw == nullptr)
                break;

            // Сбор попутного сигнала, установленного на стрелке: это сигнал той
            // ветви, по которой поезд подходит к стрелке (см. Signal::calcPosition).
            // При движении вперёд это текущая траектория и сигнал FWD стрелки;
            // при движении назад - это уже проследованная ветвь за стрелкой и тот
            // же попутный сигнал FWD, оставшийся позади поезда
            if (signal_list != nullptr)
            {
                const Signal* signal = nullptr;
                const Trajectory* signal_traj = nullptr;
                const Trajectory* passed_traj = nullptr;

                if (kind > 0)
                {
                    // Пробуем сигнал FWD на BWD-ветви (поезд подходит к стрелке
                    // со стороны BWD — штатный случай при движении «туда»)
                    signal = next_sw->getSignalFwd();
                    signal_traj = next_sw->trajectories[SW_BWD_PLUS]
                        ? next_sw->trajectories[SW_BWD_PLUS]
                        : next_sw->trajectories[SW_BWD_MINUS];
                    // Если не совпало — пробуем сигнал BWD на FWD-ветви (поезд
                    // подходит к стрелке со стороны FWD — при движении «обратно»)
                    if (!(signal != nullptr && signal_traj == traj
                        && !signal->getSignalModel().isEmpty()
                        && !signal->getSignalModel().startsWith("empty_")))
                    {
                        signal = next_sw->getSignalBwd();
                        signal_traj = next_sw->trajectories[SW_FWD_PLUS]
                            ? next_sw->trajectories[SW_FWD_PLUS]
                            : next_sw->trajectories[SW_FWD_MINUS];
                    }
                }
                else
                {
                    // Ветвь, по которой поезд уже проехал стрелку, определяем так же,
                    // как переход на следующую траекторию при обходе назад
                    dir_t passed_dir = exit_dir;
                    passed_traj = next_sw->getNextTraj(passed_dir);
                    const bool on_bwd_branch =
                        (passed_traj == next_sw->trajectories[SW_BWD_PLUS])
                        || (passed_traj == next_sw->trajectories[SW_BWD_MINUS]);
                    signal = on_bwd_branch ? next_sw->getSignalFwd()
                                           : next_sw->getSignalBwd();
                    signal_traj = on_bwd_branch
                        ? (next_sw->trajectories[SW_BWD_PLUS]
                            ? next_sw->trajectories[SW_BWD_PLUS]
                            : next_sw->trajectories[SW_BWD_MINUS])
                        : (next_sw->trajectories[SW_FWD_PLUS]
                            ? next_sw->trajectories[SW_FWD_PLUS]
                            : next_sw->trajectories[SW_FWD_MINUS]);
                }

                const Trajectory* matched_traj = (kind > 0) ? traj : passed_traj;
                if ((signal != nullptr) && (matched_traj != nullptr)
                    && (signal_traj == matched_traj)
                    && (!signal->getSignalModel().isEmpty())
                    && (!signal->getSignalModel().startsWith("empty_")))
                {
                    profile_signal_t ps;
                    ps.distance = kind * traveled;
                    ps.connector_name = signal->getConnectorName();
                    ps.signal_dir = signal->getDirection();
                    signal_list->push_back(ps);
                }
            }

            // Проверка сопряжения стрелки с ветвью входа: если состояние стрелки
            // не установлено на ветвь, по которой движется поезд, дальнейшего пути нет
            bool aligned = false;
            const auto entry_ways = (exit_dir == FWD) ? switch_bwd_ways_t
                                                      : switch_fwd_ways_t;
            for (Switch_way_t way : entry_ways)
            {
                if (next_sw->trajectories[way] != traj)
                    continue;
                Switch_state_t entry_state = (exit_dir == FWD) ? next_sw->getStateBwd()
                                                               : next_sw->getStateFwd();
                const bool is_plus = (way == SW_BWD_PLUS) || (way == SW_FWD_PLUS);
                aligned = is_plus ? (entry_state > 0) : (entry_state < 0);
                break;
            }
            if (!aligned)
                break;

            Trajectory* next_traj = next_sw->getNextTraj(exit_dir);
            if (next_traj == nullptr)
                break;
            if (exit_dir != orient)
                orient = static_cast<dir_t>(-orient);
            coord = (exit_dir == BWD) ? next_traj->getLength() : 0.0;

            close_segment();
            traj = next_traj;
            seg_begin = kind * traveled;
        }

        close_segment();
    }

    //------------------------------------------------------------------------------
    // Расчёт уклонов по высотам соседних точек профиля
    //------------------------------------------------------------------------------
    void calcInclinations(std::vector<profile_segment_t>& points)
    {
        for (size_t i = 1; i < points.size(); ++i)
        {
            double d_dist = points[i].distance - points[i - 1].distance;
            if (std::abs(d_dist) > 1e-9)
                points[i - 1].inclination = (points[i].elevation - points[i - 1].elevation) / d_dist * 1000.0;
            else
                points[i - 1].inclination = 0.0;
        }
        if (!points.empty())
            points.back().inclination = points.size() > 1 ? points[points.size() - 2].inclination : 0.0;
}

    //------------------------------------------------------------------------------
    // Ресемплинг ломаной на равномерную сетку и гауссово сглаживание высот
    //------------------------------------------------------------------------------
    void smoothProfile(std::vector<profile_segment_t>& points, size_t keep_idx,
                       double resample_step, double window_half, double sigma)
    {
        if (points.size() < 2)
            return;

        const double eps = 1e-9;
        const double keep_dist = points[keep_idx].distance;
        const double keep_elev = points[keep_idx].elevation;

        // Ресемплинг исходной ломаной на равномерную сетку
        std::vector<profile_segment_t> grid;
        grid.reserve(static_cast<size_t>((points.back().distance - points.front().distance) / resample_step) + 2);

        for (double d = points.front().distance; d <= points.back().distance + eps; d += resample_step)
        {
            double dd = std::min(d, points.back().distance);

            // Поиск отрезка исходной ломаной, содержащего dd
            size_t seg = 1;
            while (seg + 1 < points.size() && points[seg].distance < dd - eps)
                ++seg;
            const profile_segment_t& a = points[seg - 1];
            const profile_segment_t& b = points[seg];

            double t = (b.distance - a.distance) > eps
                       ? (dd - a.distance) / (b.distance - a.distance) : 0.0;
            t = std::clamp(t, 0.0, 1.0);

            profile_segment_t p;
            p.distance = dd;
            p.elevation = a.elevation + t * (b.elevation - a.elevation);
            // Километраж не интерполируем через обрыв данных (нули вместо
            // значений): иначе ресемплинг создаёт фиктивный плавный километраж
            // между реальными значениями по разные стороны обрыва
            if (a.railway_coord <= 0.5 || b.railway_coord <= 0.5)
                p.railway_coord = 0.0;
            else
                p.railway_coord = a.railway_coord + t * (b.railway_coord - a.railway_coord);
            p.inclination = 0.0;
            grid.push_back(p);

            if (dd >= points.back().distance - eps)
                break;
        }

        // Гарантируем наличие узла в точке отсчёта (distance == keep_dist)
        bool has_keep = false;
        for (const profile_segment_t& g : grid)
        {
            if (std::abs(g.distance - keep_dist) <= eps)
            {
                has_keep = true;
                break;
            }
        }
        if (!has_keep)
        {
            size_t seg = 1;
            while ((seg + 1 < grid.size()) && (grid[seg].distance < keep_dist - eps))
                ++seg;
            const profile_segment_t& a = grid[seg - 1];
            const profile_segment_t& b = grid[seg];
            double t = (b.distance - a.distance) > eps
                       ? (keep_dist - a.distance) / (b.distance - a.distance) : 0.0;
            t = std::clamp(t, 0.0, 1.0);

            profile_segment_t p;
            p.distance = keep_dist;
            p.elevation = a.elevation + t * (b.elevation - a.elevation);
            if (a.railway_coord <= 0.5 || b.railway_coord <= 0.5)
                p.railway_coord = 0.0;
            else
                p.railway_coord = a.railway_coord + t * (b.railway_coord - a.railway_coord);
            p.inclination = 0.0;
            grid.insert(grid.begin() + seg, p);
        }

        if (grid.size() < 3)
            return;

        // Гауссово сглаживание высоты по дистанции
        std::vector<double> smooth_z(grid.size());
        const double var = sigma * sigma;

        for (size_t i = 0; i < grid.size(); ++i)
        {
            double z_sum = 0.0;
            double w_sum = 0.0;
            double di = grid[i].distance;

            for (size_t j = i; j < grid.size(); ++j)
            {
                double dx = grid[j].distance - di;
                if (dx > window_half)
                    break;
                double w = std::exp(-dx * dx / (2.0 * var));
                z_sum += w * grid[j].elevation;
                w_sum += w;
            }
            for (size_t j = i; j > 0; --j)
            {
                double dx = di - grid[j - 1].distance;
                if (dx > window_half)
                    break;
                double w = std::exp(-dx * dx / (2.0 * var));
                z_sum += w * grid[j - 1].elevation;
                w_sum += w;
            }

            smooth_z[i] = z_sum / w_sum;
        }

        // Точка отсчёта и её окрестность восстанавливаем точно
        const double blend = 0.5 * window_half;

        size_t keep_grid = 0;
        double min_d = std::abs(grid[0].distance - keep_dist);
        for (size_t i = 1; i < grid.size(); ++i)
        {
            double d = std::abs(grid[i].distance - keep_dist);
            if (d < min_d)
            {
                min_d = d;
                keep_grid = i;
            }
        }

        for (size_t i = 0; i < grid.size(); ++i)
        {
            double dist = std::abs(grid[i].distance - keep_dist);
            if (dist <= blend)
            {
                double w = 1.0 - dist / blend;
                grid[i].elevation = w * keep_elev + (1.0 - w) * smooth_z[i];
            }
            else
            {
                grid[i].elevation = smooth_z[i];
            }
        }

        grid[keep_grid].elevation = keep_elev;

        points = std::move(grid);
    }

}

//------------------------------------------------------------------------------
// Профиль пути поезда: обход вперёд и назад от точки отсчёта и сглаживание
//------------------------------------------------------------------------------
bool Topology::getProfile(Trajectory* traj, double coord, dir_t orient,
                          double backward_m, double forward_m,
                          profile_segments_t& out) const
{
    if (traj == nullptr)
        return false;

    std::vector<profile_segment_t> fwd_points;
    std::vector<profile_segment_t> bwd_points;
    std::vector<profile_vehicle_t> fwd_vehicles;
    std::vector<profile_vehicle_t> bwd_vehicles;
    std::vector<profile_signal_t> fwd_signals;
    std::vector<profile_signal_t> bwd_signals;
    std::vector<profile_station_t> fwd_stations;
    std::vector<profile_station_t> bwd_stations;
    std::vector<profile_speed_limit_t> fwd_speed_limits;
    std::vector<profile_speed_limit_t> bwd_speed_limits;

    // Ход вперёд
    Trajectory* fwd_traj = traj;
    double fwd_coord = coord;
    dir_t fwd_orient = orient;
    walkProfile(fwd_traj, fwd_coord, fwd_orient, forward_m, +1, fwd_points,
                nullptr, &fwd_vehicles, &fwd_signals,
                &stations, &fwd_stations, &fwd_speed_limits);

    // Ход назад
    Trajectory* bwd_traj = traj;
    double bwd_coord = coord;
    dir_t bwd_orient = static_cast<dir_t>(-orient);
    walkProfile(bwd_traj, bwd_coord, bwd_orient, backward_m, -1, bwd_points,
                nullptr, &bwd_vehicles, &bwd_signals,
                &stations, &bwd_stations, &bwd_speed_limits);

    // Сборка профиля: назад (по убыванию) + точка отсчёта + вперёд
    out.points.clear();
    out.points.reserve(bwd_points.size() + 1 + fwd_points.size());
    for (size_t i = bwd_points.size(); i > 0; --i)
        out.points.push_back(bwd_points[i - 1]);
    out.points.push_back(pointAtCoord(traj, coord, orient, 0.0));
    out.points.insert(out.points.end(), fwd_points.begin(), fwd_points.end());

    out.backward = bwd_points.empty() ? 0.0 : std::abs(bwd_points.back().distance);
    out.forward = fwd_points.empty() ? 0.0 : fwd_points.back().distance;

    // Сборка подвижного состава на профиле: назад (по убыванию) + вперёд
    out.vehicles.clear();
    out.vehicles.reserve(bwd_vehicles.size() + fwd_vehicles.size());
    for (size_t i = bwd_vehicles.size(); i > 0; --i)
        out.vehicles.push_back(bwd_vehicles[i - 1]);
    out.vehicles.insert(out.vehicles.end(), fwd_vehicles.begin(), fwd_vehicles.end());

    // Упорядочивание по begin_distance
    std::sort(out.vehicles.begin(), out.vehicles.end(),
              [](const profile_vehicle_t& a, const profile_vehicle_t& b)
              {
                  return a.begin_distance < b.begin_distance;
              });

    // Слияние интервалов одной ПЕ, пересекающих точку отсчёта или стрелку
    // (вагон разбит на части по траекториям - объединяем в один интервал)
    std::vector<profile_vehicle_t> merged;
    merged.reserve(out.vehicles.size());
    for (const profile_vehicle_t& v : out.vehicles)
    {
        if (!merged.empty() && merged.back().vehicle_id == v.vehicle_id
            && v.begin_distance <= merged.back().end_distance + 1e-6)
        {
            merged.back().end_distance = std::max(merged.back().end_distance, v.end_distance);
        }
        else
        {
            merged.push_back(v);
        }
    }
    out.vehicles = std::move(merged);

    // Сортировка сигналов: назад (по убыванию) + вперёд, упорядочены по distance
    out.signal_list.clear();
    out.signal_list.reserve(bwd_signals.size() + fwd_signals.size());
    for (size_t i = bwd_signals.size(); i > 0; --i)
        out.signal_list.push_back(bwd_signals[i - 1]);
    out.signal_list.insert(out.signal_list.end(), fwd_signals.begin(), fwd_signals.end());
    std::sort(out.signal_list.begin(), out.signal_list.end(),
              [](const profile_signal_t& a, const profile_signal_t& b)
              {
                  return a.distance < b.distance;
              });

    // Сборка станций: назад (по убыванию) + вперёд, упорядочены по distance
    out.stations.clear();
    out.stations.reserve(bwd_stations.size() + fwd_stations.size());
    for (size_t i = bwd_stations.size(); i > 0; --i)
        out.stations.push_back(bwd_stations[i - 1]);
    out.stations.insert(out.stations.end(), fwd_stations.begin(), fwd_stations.end());
    std::sort(out.stations.begin(), out.stations.end(),
              [](const profile_station_t& a, const profile_station_t& b)
              {
                  return a.distance < b.distance;
              });

    // Сборка ограничений скорости: назад (по убыванию) + вперёд
    out.speed_limits.clear();
    out.speed_limits.reserve(bwd_speed_limits.size() + fwd_speed_limits.size());
    for (size_t i = bwd_speed_limits.size(); i > 0; --i)
        out.speed_limits.push_back(bwd_speed_limits[i - 1]);
    out.speed_limits.insert(out.speed_limits.end(), fwd_speed_limits.begin(), fwd_speed_limits.end());

    // Достраивание end_distance до начала следующего интервала
    // (гарантия непрерывности ленты ограничений)
    {
        const double eps = 1e-6;
        if (!out.speed_limits.empty())
        {
            std::sort(out.speed_limits.begin(), out.speed_limits.end(),
                      [](const profile_speed_limit_t& a, const profile_speed_limit_t& b)
                      {
                          return a.distance < b.distance;
                      });

            // Слияние одинаковых, идущих подряд
            std::vector<profile_speed_limit_t> merged;
            merged.reserve(out.speed_limits.size());
            merged.push_back(out.speed_limits[0]);
            for (size_t i = 1; i < out.speed_limits.size(); ++i)
            {
                if (std::abs(out.speed_limits[i].speed_kmh - merged.back().speed_kmh) < eps
                    && out.speed_limits[i].distance <= merged.back().end_distance + eps)
                {
                    merged.back().end_distance = std::max(merged.back().end_distance,
                                                          out.speed_limits[i].end_distance);
                }
                else
                {
                    merged.push_back(out.speed_limits[i]);
                }
            }

            // end_distance каждого интервала = начало следующего (или out.forward для последнего)
            for (size_t i = 0; i + 1 < merged.size(); ++i)
                merged[i].end_distance = merged[i + 1].distance;
            if (!merged.empty())
                merged.back().end_distance = std::max(merged.back().end_distance, out.forward);

            // Финальное слияние: соседние с одинаковой скоростью — склеиваем
            {
                std::vector<profile_speed_limit_t> final;
                final.reserve(merged.size());
                final.push_back(merged[0]);
                for (size_t i = 1; i < merged.size(); ++i)
                {
                    if (std::abs(merged[i].speed_kmh - final.back().speed_kmh) < eps)
                    {
                        final.back().end_distance = merged[i].end_distance;
                    }
                    else
                    {
                        final.push_back(merged[i]);
                    }
                }
                out.speed_limits = std::move(final);
            }
        }
    }

    // Сглаживание изломов скользящим окном для визуализации
    const size_t origin_idx = bwd_points.size();
    const double resample_step = 5.0;
    const double smooth_window = 100.0;
    const double smooth_sigma = 40.0;
    smoothProfile(out.points, origin_idx, resample_step, smooth_window, smooth_sigma);

    calcInclinations(out.points);

    return true;
}
