#include    <topology.h>

#include    <QDir>
#include    <QDirIterator>
#include    <QFile>

#include    <CfgReader.h>
#include    <switch.h>
#include    <isolated-joint.h>
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

    if (traj_list.size() == 0)
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

    for (size_t i = 0; i < vehicles->size(); ++i)
    {
        VehicleController *vc = new VehicleController;
        //vehicle_control[i] = new VehicleController;

        // Смещаем координату центра данной ПЕ
        // на половину её длины и половину длины предыдущей ПЕ
        double L = (*vehicles)[i]->getLength();
        traj_coord = traj_coord - tp.dir * L / 2.0;
        if (i != 0)
            traj_coord = traj_coord - tp.dir * (*vehicles)[i-1]->getLength() / 2.0;

        // Если траекторная координата превысила длину траектории
        // (заехали за стык или стрелку спереди), пока она её превышает...
        while (traj_coord > cur_traj->getLength())
        {
            // Получаем указатель на коннектор спереди
            Connector *conn = cur_traj->getFwdConnector();
            if (conn == nullptr)
            {
                Journal::instance()->error("Trajectory " + cur_traj->getName() + " has't forward connector");
                return false;
            }

            // Получаем указатель на следующую траекторию спереди
            Trajectory *next_traj = conn->getFwdTraj();
            if (next_traj == nullptr)
            {
                Journal::instance()->error("Connector " + conn->getName() + " has't forward trajectory");
                return false;
            }

            // Вычитаем из траекторной координаты длину предыдущей траектории,
            // чтобы получить координату на новой траектории впереди
            traj_coord = traj_coord - cur_traj->getLength();

            // Обновляем текущую траекторию на ту,
            // с которой нас соединяет коннектор спереди
            cur_traj = next_traj;
        }

        // Если траекторная координата меньше нуля
        // (заехали за стык или стрелку сзади), пока она меньше нуля...
        while (traj_coord < 0.0)
        {
            // Получаем указатель на коннектор сзади
            Connector *conn = cur_traj->getBwdConnector();
            if (conn == nullptr)
            {
                Journal::instance()->error("Trajectory " + cur_traj->getName() + " has't backward connector");
                return false;
            }

            // Получаем указатель на следующую траекторию сзади
            Trajectory *next_traj = conn->getBwdTraj();
            if (next_traj == nullptr)
            {
                Journal::instance()->error("Connector " + conn->getName() + " has't backward trajectory");
                return false;
            }

            // Добавляем к траекторной координате длину новой траектории,
            // чтобы получить координату на новой траектории сзади
            traj_coord = traj_coord + next_traj->getLength();

            // Обновляем текущую траекторию на ту,
            // с которой нас соединяет коннектор сзади
            cur_traj = next_traj;
        }

        size_t idx = vehicle_control.size();
        if ((*vehicles)[i]->getModelIndex() != idx)
        {
            Journal::instance()->warning(QString(
                "Sizes of vehicles array at model and at topology are different."));
            Journal::instance()->warning(QString(
                "For vehicle [%1] index from topology vehicle controller [%2] will be used.")
                                             .arg((*vehicles)[i]->getModelIndex())
                                             .arg(idx));

            (*vehicles)[i]->setModelIndex(idx);
        }
        vc->setIndex(idx);
        vc->setLength(L);
        vc->setVehicleRailwayConnectors((*vehicles)[i]->getRailwayConnectors());
        vc->setInitCurrentTraj(cur_traj, traj_coord);
        vc->setDirection(tp.dir);
        vc->setInitCoord((*vehicles)[i]->getTrainCoord());

        vehicle_control.push_back(vc);
        vc_table[(*vehicles)[i]] = vc;

        Journal::instance()->info(QString("Vehcile #%1").arg(idx) +
                                  " at traj: " + cur_traj->getName() +
                                  QString(" %1 m from start").arg(traj_coord));
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
        path.dir = 0;
        path.trajectories = {start_traj};
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
    std::queue<std::pair<Trajectory *, int>> q;
    // Хеш-таблица поесещенных траекторий: ключ - текущая траектория,
    // значение - предыдущая траектория.
    // Используется для восстановления пути по завершении поиска
    std::unordered_map<Trajectory *, Trajectory *> visited;

    // Начинаем с исходной траектории
    q.push({start_traj, dir});
    // Метим её как посещенную из несуществующей траектории через неизвестный узел
    visited[start_traj] = nullptr;

    // Пока очередь траекторий для посещения не пуста
    while (!q.empty())
    {
        // извлекаем текущую траекторию и направление из очереди
        auto [curr_t, d] = q.front();
        q.pop();

        // Если текущая траектория - целевая, то ура, мы нашли путь!
        if (curr_t == target_traj)
        {
            // Построенный маршрут
            route_segment_t path;
            path.dir = d;

            // Начинаем с целевой траектории
            Trajectory *t = target_traj;

            // Пока существует предыдущая траектория
            while (t != nullptr)
            {
                // Помещаем сегмент маршрута в путь
                path.trajectories.push_back(t);

                // Извлекаем предыдущую траекторию, переходим к ней
                t = visited[t];
            }

            // Инвертируем маршрут, чтобы был от начала к концу
            std::reverse(path.trajectories.begin(), path.trajectories.end());

            // Уходим, довольные как слон, с маршрутом под мышкой
            return path;
        }

        // В зависимости от направления берем либо передний, либо задний
        // коннектор текущей траектории
        Connector *next_conn = (d == 1) ? curr_t->getFwdConnector() : curr_t->getBwdConnector();

        // Если коннектора нет - мы пришли в тупик, дальше хода нет
        if (next_conn == nullptr)
        {
            // идем на следующую итерацию
            continue;
        }

        // Смотрим, какая траектория следующая
        Trajectory *next_traj = nullptr;

        // Смотрим, стрелка ли наш коннектор (Бу-гага, он всегда стрелка!)
        if (Switch *sw = dynamic_cast<Switch *>(next_conn))
        {
            // Если стрелка уже занята подвижным составом или маршрутом, дальше хода нет
            /*if ((sw->getStateFwd() == Switch::IN_ROUTE_MINUS) ||
                (sw->getStateFwd() == Switch::IS_BUSY_MINUS) ||
                (sw->getStateFwd() == Switch::IS_BUSY_PLUS) ||
                (sw->getStateFwd() == Switch::IN_ROUTE_PLUS) ||
                (sw->getStateBwd() == Switch::IN_ROUTE_MINUS) ||
                (sw->getStateBwd() == Switch::IS_BUSY_MINUS) ||
                (sw->getStateBwd() == Switch::IS_BUSY_PLUS) ||
                (sw->getStateBwd() == Switch::IN_ROUTE_PLUS))*/
            if ((sw->getStateFwd() < -1) || (sw->getStateFwd() > 1) ||
                (sw->getStateBwd() < -1) || (sw->getStateBwd() > 1))
            {
                // идем на следующую итерацию
                continue;
            }

            // Список траекторий кандидатов в высокое звание маршрутных
            std::vector<Trajectory *> candidates;

            // Если едем вперед
            if (d == 1)
            {
                // Если есть прямое по стрелке направление
                // и оно не занято и не включено в другой маршрут
                if (sw->fwdPlusTraj && !sw->fwdPlusTraj->isBusy() && !sw->fwdPlusTraj->isInRoute())
                {
                    // то это наш кандидат
                    candidates.push_back(sw->fwdPlusTraj);
                }

                // Если есть незанатое направление по отклонению не включенной в другой маршрут
                if (sw->fwdMinusTraj && !sw->fwdMinusTraj->isBusy() && !sw->fwdMinusTraj->isInRoute())
                {
                    // То же кандидат, надо рассмотреть!
                    candidates.push_back(sw->fwdMinusTraj);
                }
            }
            else // при движении назад - смотрим на задние +/- хвостики стрелки
            {
                // так же добавляя в кандидаты существующие незанятые траектории
                if (sw->bwdPlusTraj && !sw->bwdPlusTraj->isBusy() && !sw->bwdPlusTraj->isInRoute())
                {
                    candidates.push_back(sw->bwdPlusTraj);
                }

                if (sw->bwdMinusTraj && !sw->bwdMinusTraj->isBusy() && !sw->bwdMinusTraj->isInRoute())
                {
                    candidates.push_back(sw->bwdMinusTraj);
                }
            }

            // Перебираем собранных траекторий-кандидатов
            for (Trajectory *cand : candidates)
            {
                // Если он еще не посещён - то список посещённых вернет end()
                if (visited.find(cand) == visited.end())
                {
                    // Запоминаем кандидата как посещённого, и откуда мы к нему пришли
                    visited[cand] = curr_t;
                    // и помещаем его в очередь
                    q.push({cand, d});
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
std::pair<Trajectory *, Trajectory *> Topology::check_build_route_command(const route_command_t &rc)
{
    if ((rc.dir != 1) && (rc.dir != -1))
    {
        Journal::instance()->error("BuildRoute: Invalid direction of searching "
                                   + QString::number(rc.dir));
        return {nullptr, nullptr};
    }

    auto s_traj = traj_list.value(rc.trajectory_begin, nullptr);

    if (s_traj == nullptr)
    {
        Journal::instance()->error("BuildRoute: Unknown start trajectory "
                                   + rc.trajectory_begin);
        return {nullptr, nullptr};
    }

    auto t_traj = traj_list.value(rc.trajectory_end, nullptr);

    if (t_traj == nullptr)
    {
        Journal::instance()->error("BuildRoute: Unknown target trajectory "
                                   + rc.trajectory_end);
        return {nullptr, nullptr};
    }

    return {s_traj, t_traj};
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::set_switchs_by_route(const route_segment_t& route, int dir)
{
    for (size_t i = 0; i < route.trajectories.size() - 1; ++i)
    {
        // Берём очередную траекторию маршрута
        Trajectory* prev_traj = route.trajectories[i];

        // Берем коннектор у траектории в направлении построенного маршрута
        Connector* conn = (dir == 1) ? prev_traj->getFwdConnector() : prev_traj->getBwdConnector();
        if (conn == nullptr)
        {
            Journal::instance()->error(QString("Set switches by route: %1 conn of [%2]%3 is null")
                                           .arg((dir == 1) ? "Fwd" : "Bwd").arg(i).arg(prev_traj->getName()));
            return false;
        }

        Switch* sw = dynamic_cast<Switch*>(conn);
        if (sw == nullptr)
        {
            Journal::instance()->error(QString("Set switches by route: %1 conn of [%2]%3 is not a switch")
                                           .arg((dir == 1) ? "Fwd" : "Bwd").arg(i).arg(prev_traj->getName()));
            return false;
        }

        // Ожидаемая траектория, исходя из построения маршрута
        Trajectory* next_traj = route.trajectories[i + 1];

        if (dir == 1)
        {
            // Переключаем попутные остряки
            if (next_traj == sw->fwdPlusTraj)
            {
                sw->setRefStateFwd(Switch::STATE_PLUS);
            }

            if (next_traj == sw->fwdMinusTraj)
            {
                sw->setRefStateFwd(Switch::STATE_MINUS);
            }

            // Переключаем встречные остряки
            if (prev_traj == sw->bwdPlusTraj)
            {
                sw->setRefStateBwd(Switch::STATE_PLUS);
            }

            if (prev_traj == sw->bwdMinusTraj)
            {
                sw->setRefStateBwd(Switch::STATE_MINUS);
            }
        }

        if (dir == -1)
        {
            // Переключаем попутные остряки
            if (next_traj == sw->bwdPlusTraj)
            {
                sw->setRefStateBwd(Switch::STATE_PLUS);
            }

            if (next_traj == sw->bwdMinusTraj)
            {
                sw->setRefStateBwd(Switch::STATE_MINUS);
            }

            // Переключаем встречные остряки
            if (prev_traj == sw->fwdPlusTraj)
            {
                sw->setRefStateFwd(Switch::STATE_PLUS);
            }

            if (prev_traj == sw->fwdMinusTraj)
            {
                sw->setRefStateFwd(Switch::STATE_MINUS);
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::open_route_signals(const route_segment_t &route, int dir, QStringList &conn_list)
{
    for (size_t i = 0; i < route.trajectories.size() - 1; ++i)
    {
        // Берём очередную траекторию маршрута
        Trajectory* traj = route.trajectories[i];

        // Берем коннектор у траектории в направлении построенного маршрута
        Connector *conn = (dir == 1) ? traj->getFwdConnector() : traj->getBwdConnector();

        if (conn == nullptr)
        {
            Journal::instance()->error(QString("Open route signals: %1 conn of [%2]%3 is null")
                                           .arg((dir == 1) ? "Fwd" : "Bwd").arg(i).arg(traj->getName()));
            return false;
        }

        // Проверяем есть ли на нем сигнал
        Signal* signal = (dir == 1) ? conn->getSignalFwd() : conn->getSignalBwd();

        if (signal == nullptr)
        {
            // нет, и открывать нечего, идем дальше
            continue;
        }

        if (StationSignal* station_sig = dynamic_cast<StationSignal *>(signal))
        {
            conn_list.append(conn->getName());
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

    for (auto conn = joints.begin(); conn != joints.end(); ++conn)
    {
        (*conn)->step(t, dt);
    }

    for (auto conn = switches.begin(); conn != switches.end(); ++conn)
    {
        (*conn)->step(t, dt);
    }

    for (auto& signals_array : {signals_data.line_signals,
                                signals_data.enter_signals,
                                signals_data.route_signals,
                                signals_data.exit_signals,
                                signals_data.shunt_signals})
    {
        for (auto& signal : signals_array)
        {
            if (signal == nullptr)
            {
                continue;
            }

            signal->step(t, dt);
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

    for (auto station : stations)
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

    secNode = cfg.getFirstSection("Joint");

    while (!secNode.isNull())
    {
        IsolatedJoint *joint = new IsolatedJoint();
        joint->configure(cfg, secNode, traj_list);

        joints.insert(joint->getName(), joint);

        secNode = cfg.getNextSection();
    }

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
void Topology::load_signals(CfgReader &cfg, QDomNode secNode, Connector *conn)
{
    QString signal_model_fwd = "";
    int signal_dir_fwd = 0;

    QString signal_model_bwd = "";
    int signal_dir_bwd = 0;

    if (cfg.getString(secNode, "SignalModelFwd", signal_model_fwd))
    {
        signal_dir_fwd = 1;
    }

    if (cfg.getString(secNode, "SignalModelBwd", signal_model_bwd))
    {
        signal_dir_bwd = -1;
    }

    auto configure_signal = [](Signal* signal, Connector* conn, int direction,
                               QString signal_letter, QString signal_model,
                               dvec3 relative_position, dvec3 relative_rotation)
    {
        if (direction == 1)
            conn->setSignalFwd(signal);
        if (direction == -1)
            conn->setSignalBwd(signal);

        signal->setConnector(conn);
        signal->setDirection(direction);
        signal->setLetter(signal_letter);
        signal->setSignalModel(signal_model);
        signal->setRelPosition(relative_position);
        signal->setRelRotation(relative_rotation);
    };

    if (signal_dir_fwd == 1)
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
            configure_signal(signal, conn, signal_dir_fwd,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.line_signals.push_back(signal);
            Journal::instance()->info("Loaded line signal " + signal->getLetter());
        }

        if (signal_model_fwd.right(4) == "entr")
        {
            EnterSignal *signal = new EnterSignal;
            configure_signal(signal, conn, signal_dir_fwd,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.enter_signals.push_back(signal);
            Journal::instance()->info("Loaded enter signal " + signal->getLetter());
        }

        if (signal_model_fwd.right(4) == "rout")
        {
            RouteSignal *signal = new RouteSignal;
            configure_signal(signal, conn, signal_dir_fwd,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.route_signals.push_back(signal);
            Journal::instance()->info("Loaded route signal " + signal->getLetter());
        }

        if (signal_model_fwd.right(4) == "exit")
        {
            ExitSignal *signal = new ExitSignal;
            configure_signal(signal, conn, signal_dir_fwd,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.exit_signals.push_back(signal);
            Journal::instance()->info("Loaded exit signal " + signal->getLetter());
        }

        if (signal_model_fwd.right(4) == "shnt")
        {
            ShuntingSignal *signal = new ShuntingSignal;
            configure_signal(signal, conn, signal_dir_fwd,
                             signal_letter, signal_model_fwd,
                             rel_pos, rel_rot);
            signals_data.shunt_signals.push_back(signal);
            Journal::instance()->info("Loaded shunting signal " + signal->getLetter());
        }
    }

    if (signal_dir_bwd == -1)
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
            configure_signal(signal, conn, signal_dir_bwd,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.line_signals.push_back(signal);
            Journal::instance()->info("Loaded line signal " + signal->getLetter());
        }

        if (signal_model_bwd.right(4) == "entr")
        {
            EnterSignal *signal = new EnterSignal;
            configure_signal(signal, conn, signal_dir_bwd,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.enter_signals.push_back(signal);
            Journal::instance()->info("Loaded enter signal " + signal->getLetter());
        }

        if (signal_model_bwd.right(4) == "rout")
        {
            RouteSignal *signal = new RouteSignal;
            configure_signal(signal, conn, signal_dir_bwd,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.route_signals.push_back(signal);
            Journal::instance()->info("Loaded route signal " + signal->getLetter());
        }

        if (signal_model_bwd.right(4) == "exit")
        {
            ExitSignal *signal = new ExitSignal;
            configure_signal(signal, conn, signal_dir_bwd,
                             signal_letter, signal_model_bwd,
                             rel_pos, rel_rot);
            signals_data.exit_signals.push_back(signal);
            Journal::instance()->info("Loaded exit signal " + signal->getLetter());
        }

        if (signal_model_bwd.right(4) == "shnt")
        {
            ShuntingSignal *signal = new ShuntingSignal;
            configure_signal(signal, conn, signal_dir_bwd,
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
void Topology::serialize_connector_name(QDataStream &stream, Connector *conn)
{
    if (bool has_conn = conn != nullptr)
    {
        stream << has_conn;
        stream << conn->getName();
    }
    else
    {
        stream << has_conn;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Connector *Topology::deserialize_traj_connectors(QDataStream &stream, conn_list_t &conn_list) const
{
    bool has_conn = false;
    stream >> has_conn;

    if (has_conn)
    {
        QString conn_name = "";
        stream >> conn_name;

        return conn_list.value(conn_name, nullptr);
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
        sw->setRefStateBwd(static_cast<Switch::State>(sc.switch_ref_state));
    }
    else
    {
        sw->setRefStateFwd(static_cast<Switch::State>(sc.switch_ref_state));
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

    Connector* conn = switches.value(sc.conn_name, nullptr);
    if (conn == nullptr)
    {
        return;
    }

    Signal* sig = (sc.sig_dir < 1) ? conn->getSignalBwd() : conn->getSignalFwd();
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

    auto [s_traj, t_traj] = check_build_route_command(rc);
    if ((s_traj == nullptr) || (t_traj == nullptr))
    {
        return;
    }

    auto route = find_route(s_traj, t_traj, rc.dir);

    if (route.trajectories.empty())
    {
        Journal::instance()->error("Build route: No route from "
                                   + rc.trajectory_begin + " to " + rc.trajectory_end);
        return;
    }
    Journal::instance()->info("Build route: founded from "
                              + rc.trajectory_begin + " to " + rc.trajectory_end
                              + " through " + QString::number(route.trajectories.size()) + "trajectories");

    if (!set_switchs_by_route(route, rc.dir))
    {
        Journal::instance()->error("Build route: Route switches cannot be set");
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotTrainRouteCommand(QByteArray &route_data)
{
    route_command_t rc;
    rc.deserialize(route_data);

    auto [s_traj, t_traj] = check_build_route_command(rc);
    if ((s_traj == nullptr) || (t_traj == nullptr))
    {
        return;
    }

    auto route = find_route(s_traj, t_traj, rc.dir);

    if (route.trajectories.empty())
    {
        Journal::instance()->error("Build route: No route from "
                                   + rc.trajectory_begin + " to " + rc.trajectory_end);
        return;
    }
    Journal::instance()->info("Build route: founded from "
                              + rc.trajectory_begin + " to " + rc.trajectory_end
                              + " through " + QString::number(route.trajectories.size()) + "trajectories");

    if (!set_switchs_by_route(route, rc.dir))
    {
        Journal::instance()->error("Build route: Route switches cannot be set");
        return;
    }

    QStringList signals_for_open;
    if (!open_route_signals(route, rc.dir, signals_for_open))
    {
        Journal::instance()->error("Build route: Can't open route signals");
    }

    emit sigSetOpenSignalsQueue(signals_for_open, rc.dir);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotShuntingRouteCommand(QByteArray &route_data)
{
    // TODO
    slotTrainRouteCommand(route_data);
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
