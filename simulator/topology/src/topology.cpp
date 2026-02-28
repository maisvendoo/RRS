#include    <topology.h>

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
Topology::~Topology()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::load(QString route_dir, bool solve_errors)
{
    FileSystem &fs = FileSystem::getInstance();

    QString route_path = QString(fs.getRouteRootDir().c_str()) +
                         QDir::separator() + route_dir;

    QStringList names = getTrajNamesList(route_path);

    if (names.isEmpty())
    {
        Journal::instance()->error("TRAJECTORIES NOT FOUND!!!");
        return false;
    }

    std::vector<std::vector<module_cfg_t>> all_modules = load_topology_configs(route_path);

    for (auto it = names.begin(); it != names.end(); ++it)
    {
        QString name = *it;
        Trajectory *traj = new Trajectory();

        std::vector<module_cfg_t> modules;
        for (auto all_cfgs = all_modules.begin(); all_cfgs != all_modules.end(); ++all_cfgs)
        {
            for (auto it = all_cfgs->begin(); it != all_cfgs->end(); ++it)
            {
                module_cfg_t mc = *it;
                if (mc.traj_names.contains(name))
                {
                    modules.push_back(mc);
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
    Trajectory *cur_traj = traj_list.value(tp.traj_name, nullptr);
    if (cur_traj == nullptr)
    {
        Journal::instance()->critical("INVALID INITIAL TRAJECTORY!!!");
        return false;
    }

    double traj_coord = std::clamp(tp.traj_coord, 0.0, cur_traj->getLength());

    dir_t dir = (tp.dir < 0) ? (BWD) : (FWD);
    for (size_t i = 0; i < vehicles->size(); ++i)
    {
        VehicleController *vc = new VehicleController;
        //vehicle_control[i] = new VehicleController;
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
            return false;
        }
   }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleController *Topology::getVehicleController(size_t idx)
{
    return vehicle_control[idx];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
route_segment_t Topology::find_route(Trajectory *start_traj,
                                     Trajectory *target_traj,
                                     int dir)
{
    // Маршрут на самого себя
    if (start_traj == target_traj)
    {
        route_segment_t path;
        path.trajectories = {start_traj};
        path.directions = {FWD};
        Journal::instance()->warning("Build route: Target trajectory and start trajectory are the same");
        return path;
    }

    // Если целевая траектория занята, уходим сразу, ловить нечего
    if (target_traj->isBusy())
    {
        Journal::instance()->error("Build route: Target trajectory is busy. Route is impossible");
        return route_segment_t();
    }

    // Очередь для обхода графа (пары текущая траектория и направление)
    std::queue<std::pair<Trajectory *, dir_t>> q;
    // Хеш-таблица посещённых траекторий: ключ - текущая траектория,
    // значение - предыдущая траектория и предыдущее направление.
    // Используется для восстановления пути по завершении поиска
    std::unordered_map<Trajectory *, std::pair<Trajectory *, dir_t>> visited;

    // Начинаем с исходной траектории
    q.push({start_traj, (dir < 0) ? BWD : FWD});
    // Метим её как посещенную из несуществующей траектории через неизвестный узел
    visited[start_traj] = {nullptr, FWD};

    // Пока очередь траекторий для посещения не пуста
    while (!q.empty())
    {
        // извлекаем текущую траекторию и направление из очереди
        const auto& [curr_t, d] = q.front();
        q.pop();

        // Если текущая траектория - целевая, то ура, мы нашли путь!
        if (curr_t == target_traj)
        {
            // Построенный маршрут
            route_segment_t path;

            // Начинаем с целевой траектории
            Trajectory *path_t = target_traj;
            dir_t path_d = d;

            // Пока существует предыдущая траектория
            while (path_t != nullptr)
            {
                // Помещаем сегмент маршрута в путь
                path.trajectories.push_back(path_t);
                path.directions.push_back(path_d);

                // Извлекаем предыдущую траекторию, переходим к ней
                std::tie(path_t, path_d) = visited[path_t];
            }

            // Инвертируем маршрут, чтобы был от начала к концу
            std::reverse(path.trajectories.begin(), path.trajectories.end());
            std::reverse(path.directions.begin(), path.directions.end());

            // Уходим, довольные как слон, с маршрутом под мышкой
            return path;
        }

        dir_t next_d = d;

        // В зависимости от направления берем либо передний, либо задний
        // коннектор текущей траектории
        Switch* next_sw = curr_t->getNextSwitch(next_d);

        // Если коннектора нет - мы пришли в тупик, дальше хода нет
        if (next_sw == nullptr)
        {
            // идем на следующую итерацию
            continue;
        }

        // Если стрелка уже занята подвижным составом или маршрутом, дальше хода нет
        if (   (next_sw->getStateFwd() == IN_ROUTE_MINUS)
            || (next_sw->getStateFwd() == IS_BUSY_MINUS)
            || (next_sw->getStateFwd() == IS_BUSY_PLUS)
            || (next_sw->getStateFwd() == IN_ROUTE_PLUS)
            || (next_sw->getStateBwd() == IN_ROUTE_MINUS)
            || (next_sw->getStateBwd() == IS_BUSY_MINUS)
            || (next_sw->getStateBwd() == IS_BUSY_PLUS)
            || (next_sw->getStateBwd() == IN_ROUTE_PLUS))
        {
            // идем на следующую итерацию
            continue;
        }

        // Список траекторий кандидатов в высокое звание маршрутных
        std::vector<Trajectory *> candidates;

        // Если едем по стрелке вперёд
        if (next_d == FWD)
        {
            // Перебираем пути вперёд
            for (const Switch_way_t& way : switch_fwd_ways_t)
            {
                if (Trajectory* traj = next_sw->trajectories[way])
                {
                    // Если траектория занята или включена в другой маршрут,
                    // не рассматриваем маршрут через них
                    if (traj->isBusy() || traj->isInRoute())
                    {
                        continue;
                    }

                    // Если траектория еще не посещалась - то список посещённых вернет end()
                    if (visited.find(traj) == visited.end())
                    {
                        // Запоминаем траекторию как посещённую, и откуда мы к нему пришли
                        visited[traj] = {curr_t, d};

                        // Направление по новой траектории
                        dir_t traj_d = static_cast<dir_t>(next_d * next_sw->getTrajOrientation(traj));
                        // и помещаем новую траекторию в очередь
                        q.push({traj, traj_d});
                    }
                }
            }
        }

        // Если едем по стрелке назад
        if (next_d == BWD)
        {
            // Перебираем пути назад
            for (const Switch_way_t& way : switch_bwd_ways_t)
            {
                if (Trajectory* traj = next_sw->trajectories[way])
                {
                    // Если траектория занята или включена в другой маршрут,
                    // не рассматриваем маршрут через них
                    if (traj->isBusy() || traj->isInRoute())
                    {
                        continue;
                    }

                    // Если траектория еще не посещалась - то список посещённых вернет end()
                    if (visited.find(traj) == visited.end())
                    {
                        // Запоминаем траекторию как посещённую, и откуда мы к нему пришли
                        visited[traj] = {curr_t, d};

                        // Направление по новой траектории
                        dir_t traj_d = static_cast<dir_t>(next_d * next_sw->getTrajOrientation(traj));
                        // и помещаем новую траекторию в очередь
                        q.push({traj, traj_d});
                    }
                }
            }
        }
    }

    // Пичалька - очередь пуста, а маршрута нет :(
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
bool Topology::open_route_signals(const route_segment_t& route, QStringList& sw_list, bool for_train)
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
                sw_list.append(sw->getName());
            }
        }
        else
        {
            // Добавляем в список
            sw_list.append(sw->getName());
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::step(double t, double dt)
{
    for (auto traj = traj_list.begin(); traj != traj_list.end(); ++traj)
    {
        (*traj)->clearBusy();
    }

    for (auto vc = vehicle_control.begin(); vc != vehicle_control.end(); ++vc)
    {
        (*vc)->step(t, dt);
    }

    for (auto traj = traj_list.begin(); traj != traj_list.end(); ++traj)
    {
        (*traj)->step(t, dt);
    }

    for (auto sw = switches.begin(); sw != switches.end(); ++sw)
    {
        (*sw)->step(t, dt);
    }

    for (auto& signals_array : {signals_data.line_signals,
                                signals_data.enter_signals,
                                signals_data.route_signals,
                                signals_data.exit_signals,
                                signals_data.shunt_signals})
    {
        for (auto* signal : signals_array)
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
    // Задаем буфер для данных и открываем его на запись
    QBuffer data;
    data.open(QIODevice::WriteOnly);
    // Связываем с буфером поток данных
    QDataStream stream(&data);

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
    for (auto traj = traj_list.begin(); traj != traj_list.end(); ++traj)
    {
        stream << traj.value()->serialize();
    }

    // Указываем число коннекторов
    stream << static_cast<uint32_t>(switches.size());

    // Складываем в буфер сериализованную информацию о коннекторах
    for (auto sw = switches.begin(); sw != switches.end(); ++sw)
    {
        stream << sw.value()->serialize();
    }

    return data.data();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::deserialize(QByteArray &data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

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
    QString topology_path = route_path + QDir::separator() + "topology";
    QDir topology_dir = QDir(topology_path);
    QStringList traj_modules_dirs = topology_dir.entryList({"trajectory-*"}, QDir::Dirs);

    // Из папок trajectory-* загружаем все конфиги *.xml
    std::vector<std::vector<module_cfg_t>> all_modules;
    for (auto name : traj_modules_dirs)
    {
        if (name.isEmpty())
            continue;

        QString traj_module_path = topology_path + QDir::separator() + name;
        QDir traj_module_dir = QDir(traj_module_path);
        QStringList cfg_files = traj_module_dir.entryList({"*.xml"}, QDir::Files);

        std::vector<module_cfg_t> all_cfgs;
        for (auto cfg_name : cfg_files)
        {
            if (cfg_name.isEmpty())
                continue;

            module_cfg_t mc;

            QString cfg_path = traj_module_path + QDir::separator() + cfg_name;
            if (!mc.cfg.load(cfg_path))
                continue;

            mc.module_name = name;

            // Список траекторий в этом конфиге:
            // модуль будет подгружен к траекториям,
            // имя которой указано хотя бы в одном конфиге,
            // после чего настроен этим же конфигом
            QDomNode trajNode = mc.cfg.getFirstSection("Trajectory");
            while (!trajNode.isNull())
            {
                QString traj_name;
                mc.cfg.getString(trajNode, "Name", traj_name);

                if (traj_name.isEmpty())
                    Journal::instance()->warning("Empty trajectory name at " + cfg_path);
                else
                    mc.traj_names.push_back(traj_name);

                trajNode = mc.cfg.getNextSection();
            }

            if (mc.traj_names.empty())
                Journal::instance()->warning("No trajectories found in " + cfg_path);
            else
                all_cfgs.push_back(mc);
        }

        if (all_cfgs.empty())
            Journal::instance()->warning("No trajectories found in files at " + traj_module_path);
        else
            all_modules.push_back(all_cfgs);
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
            sw->setSignalFwd(signal);
        if (direction == BWD)
            sw->setSignalBwd(signal);

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
                              + " through " + QString::number(route.trajectories.size()) + "trajectories");

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
                              + " through " + QString::number(route.trajectories.size()) + "trajectories");

    if (set_switchs_by_route(route))
    {
        QStringList signals_for_open;
        open_route_signals(route, signals_for_open, true);

        emit sigSetOpenSignalsQueue(signals_for_open, rc.dir, true, false);
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
                              + " through " + QString::number(route.trajectories.size()) + "trajectories");

    if (set_switchs_by_route(route))
    {
        QStringList signals_for_open;
        open_route_signals(route, signals_for_open, false);

        emit sigSetOpenSignalsQueue(signals_for_open, rc.dir, false, true);
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

    auto route_seg = find_route(start_traj, end_traj, dir);

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
void Topology::slotGetRouteLength(QString cur_traj_name, double cur_coord,
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

    auto route_seg = find_route(cur_traj, target_traj, dir);

    if (route_seg.trajectories.empty())
    {
        *lenght = -1;
        return;
    }

    if (route_seg.directions[0] == FWD)
    {
        *lenght = route_seg.trajectories[0]->getLength() - cur_coord;
    }

    if (route_seg.directions[0] == BWD)
    {
        *lenght = cur_coord;
    }

    for (size_t i = 1; i < route_seg.trajectories.size() - 1; ++i)
    {
        *lenght += route_seg.trajectories[i]->getLength();
    }

    if (route_seg.directions.back() == FWD)
    {
        *lenght += target_coord;
    }

    if (route_seg.directions.back() == BWD)
    {
        *lenght += route_seg.trajectories.back()->getLength() - target_coord;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotTrajChangeState(int vehicle_idx, bool is_busy, QString traj_name)
{
    // Определяем поезд, изменивший состояние траектории
    size_t train_idx = vehicle_control[vehicle_idx]->getTrainIndex();

    emit sigChangeTrajStateByTrain(static_cast<int>(train_idx), is_busy, traj_name);
}
