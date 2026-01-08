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

    line_signals_connect(signals_data.line_signals);

    enter_signal_connect(signals_data.enter_signals);

    //line_signals_connect(signals_data.exit_signals);

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
void Topology::line_signals_step(double t, double dt)
{
    for (auto line_signal : signals_data.line_signals)
    {
        bool is_busy = false;

        if (line_signal->getDirection() == 1)
        {
            Connector *conn = line_signal->getConnector();

            if (conn == nullptr)
            {
                continue;

            }

            Trajectory *traj = conn->getBwdTraj();

            if (traj == nullptr)
            {
                continue;
            }

            is_busy = traj->isBusy();
        }

        if (line_signal->getDirection() == -1)
        {
            Connector *conn = line_signal->getConnector();

            if (conn == nullptr)
            {
                continue;
            }

            Trajectory *traj = conn->getFwdTraj();

            if (traj == nullptr)
            {
                continue;
            }

            is_busy = traj->isBusy();
        }

        line_signal->setBusy(is_busy);
        line_signal->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::enter_signals_step(double t, double dt)
{
    for (auto signal : signals_data.enter_signals)
    {
        EnterSignal *es = dynamic_cast<EnterSignal *>(signal);

        if (es == nullptr)
        {
            continue;
        }

        if (es->getDirection() == 1)
        {
            Connector *conn = es->getConnector();

            if (conn == nullptr)
            {
                continue;

            }

            Trajectory *bwd_traj = conn->getBwdTraj();
            Trajectory *fwd_traj = conn->getFwdTraj();

            if ( (bwd_traj == nullptr) || (fwd_traj == nullptr) )
            {
                continue;
            }

            es->setFwdBusy(fwd_traj->isBusy());
            es->setBwdBusy(bwd_traj->isBusy());
        }

        if (es->getDirection() == -1)
        {
            Connector *conn = es->getConnector();

            if (conn == nullptr)
            {
                continue;

            }

            Trajectory *bwd_traj = conn->getFwdTraj();
            Trajectory *fwd_traj = conn->getBwdTraj();

            if ( (bwd_traj == nullptr) || (fwd_traj == nullptr) )
            {
                continue;
            }

            es->setFwdBusy(fwd_traj->isBusy());
            es->setBwdBusy(bwd_traj->isBusy());
        }

        es->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::exit_signals_step(double t, double dt)
{
    for (auto signal : signals_data.exit_signals)
    {
        if (signal == nullptr)
        {
            continue;
        }

        signal->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::optional<std::vector<route_segment_t> > Topology::find_route(Trajectory *start_traj,
                                                                 Trajectory *target_traj,
                                                                 int dir)
{
    std::queue<std::pair<Trajectory *, int>> q;
    std::unordered_map<Trajectory *, std::pair<Trajectory *, Connector *>> visited;

    q.push({start_traj, dir});
    visited[start_traj] = {nullptr, nullptr};

    while (!q.empty())
    {
        auto [curr_t, d] = q.front();
        q.pop();

        if (curr_t == target_traj)
        {
            std::vector<route_segment_t> path;
            Trajectory *t = target_traj;

            while (t != nullptr)
            {
                auto [prev_t, conn] = visited[t];
                route_segment_t rs;
                rs.traj = t;
                rs.dir = d;
                rs.next_conn = conn;

                path.push_back(rs);

                t = prev_t;
            }

            std::reverse(path.begin(), path.end());

            return path;
        }

        Connector *next_conn = (d == 1) ? curr_t->getFwdConnector() : curr_t->getBwdConnector();

        if (next_conn == nullptr)
        {
            continue;
        }

        Trajectory *next_traj = nullptr;

        if (Switch *sw = dynamic_cast<Switch *>(next_conn))
        {
            std::vector<Trajectory *> candidates;

            if (d == 1)
            {
                if (sw->fwdPlusTraj && !sw->fwdPlusTraj->isBusy())
                    candidates.push_back(sw->fwdPlusTraj);

                if (sw->fwdMinusTraj && !sw->fwdMinusTraj->isBusy())
                    candidates.push_back(sw->fwdMinusTraj);
            }
            else
            {
                if (sw->bwdPlusTraj && !sw->bwdPlusTraj->isBusy())
                    candidates.push_back(sw->bwdPlusTraj);

                if (sw->bwdMinusTraj && !sw->bwdMinusTraj->isBusy())
                    candidates.push_back(sw->bwdMinusTraj);
            }

            for (Trajectory *cand : candidates)
            {
                if (visited.find(cand) == visited.end())
                {
                    visited[cand] = {curr_t, next_conn};
                    q.push({cand, d});
                }
            }
        }
    }

    return std::nullopt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::set_switchs_by_route(const std::vector<route_segment_t> &route, int dir)
{
    // Переключаем попутные остряки
    for (size_t i = 0; i < route.size() - 1; ++i)
    {
        // Берем первый сегмент маршрута
        auto seg = route[i];

        // Определяем следующий коннектор в соотвествии с направлением движения
        Connector *conn = nullptr;

        if (dir == 1)
        {
            conn = seg.traj->getFwdConnector();
        }

        if (dir == -1)
        {
            conn = seg.traj->getBwdConnector();
        }

        if (Switch *sw = dynamic_cast<Switch *>(conn))
        {
            // Следующая траектория, соответсвующая текущему положению стрелки
            Trajectory *next_traj = (dir == 1) ? sw->getFwdTraj() : sw->getBwdTraj();

            // Ожидаемая траектория, исходя из построения маршрута
            Trajectory *expc_traj = route[i+1].traj;

            // Если траектории совпадают - ничего не переключаем
            if (next_traj == expc_traj)
            {
                continue;
            }

            if (dir == 1)
            {
                if (expc_traj == sw->fwdPlusTraj)
                {
                    sw->setRefStateFwd(1);
                }

                if (expc_traj == sw->fwdMinusTraj)
                {
                    sw->setRefStateFwd(-1);
                }
            }

            if (dir == -1)
            {
                if (expc_traj == sw->bwdPlusTraj)
                {
                    sw->setRefStateBwd(1);
                }

                if (expc_traj == sw->bwdMinusTraj)
                {
                    sw->setRefStateBwd(-1);
                }
            }
        }
    }

    // Переключаем встречные остряки по тому же принципу, но наоборот
    for (size_t i = route.size() - 1; i > 0; --i)
    {
        auto seg = route[i];

        Connector *conn = nullptr;

        if (dir == 1)
        {
            conn = seg.traj->getBwdConnector();
        }

        if (dir == -1)
        {
            conn = seg.traj->getFwdConnector();
        }

        if (Switch *sw = dynamic_cast<Switch *>(conn))
        {
            Trajectory *next_traj = (dir == 1) ? sw->getBwdTraj() : sw->getFwdTraj();

            Trajectory *expc_traj = route[i-1].traj;

            if (next_traj == expc_traj)
            {
                continue;
            }

            if (dir == 1)
            {
                if (expc_traj == sw->bwdPlusTraj)
                {
                    sw->setRefStateBwd(1);
                }

                if (expc_traj == sw->bwdMinusTraj)
                {
                    sw->setRefStateBwd(-1);
                }
            }

            if (dir == -1)
            {
                if (expc_traj == sw->fwdPlusTraj)
                {
                    sw->setRefStateFwd(1);
                }

                if (expc_traj == sw->fwdMinusTraj)
                {
                    sw->setRefStateFwd(-1);
                }
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::open_route_signals(const std::vector<route_segment_t> &route, int dir, QStringList &conn_list)
{
    for (size_t i = 0; i < route.size() - 1; ++i)
    {
        // Берём сегмент маршрута
        const route_segment_t& seg = route[i];

        // Берем коннектор у траектории в направлении построенного маршрута
        Connector *conn = (dir == 1) ? seg.traj->getFwdConnector() : seg.traj->getBwdConnector();

        if (conn == nullptr)
        {
            Journal::instance()->error(QString("Open route signals: %1 conn of [%2]%3 is null")
                                           .arg((dir == 1) ? "Fwd" : "Bwd").arg(i).arg(seg.traj->getName()));
            return false;
        }

        // Проверяем есть ли на нем сигнал
        Signal *signal = (dir == 1) ? conn->getSignalFwd() : conn->getSignalBwd();

        if (signal == nullptr)
        {
            // нет, и открывать нечего, идем дальше
            continue;
        }

        if (EnterSignal *enter_sig = dynamic_cast<EnterSignal *>(signal))
        {
            conn_list.append(conn->getName());
        }

        if (ExitSignal *exit_sig = dynamic_cast<ExitSignal *>(signal))
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

    line_signals_step(t, dt);

    enter_signals_step(t, dt);

    exit_signals_step(t, dt);
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
            signal->setLetter(signal_letter);
            signal->setDirection(signal_dir_fwd);
            signal->setSignalModel(signal_model_fwd);
            signal->setConnector(conn);

            conn->setSignalFwd(signal);

            signal->setRelPosition(rel_pos);
            signal->setRelRotation(rel_rot);

            signals_data.line_signals.push_back(signal);

            Journal::instance()->info("Loaded line signal " + signal->getLetter());
        }

        if ( (signal_model_fwd.right(4) == "entr") || (signal_model_fwd.right(4) == "rout") )
        {
            EnterSignal *signal = new EnterSignal;
            signal->setLetter(signal_letter);
            signal->setDirection(signal_dir_fwd);
            signal->setSignalModel(signal_model_fwd);
            signal->setConnector(conn);

            conn->setSignalFwd(signal);

            signal->setRelPosition(rel_pos);
            signal->setRelRotation(rel_rot);

            signals_data.enter_signals.push_back(signal);

            Journal::instance()->info("Loaded enter signal " + signal->getLetter());
        }

        if (signal_model_fwd.right(4) == "exit")
        {
            ExitSignal *signal = new ExitSignal;
            signal->setLetter(signal_letter);
            signal->setDirection(signal_dir_fwd);
            signal->setSignalModel(signal_model_fwd);
            signal->setConnector(conn);

            conn->setSignalFwd(signal);

            signal->setRelPosition(rel_pos);
            signal->setRelRotation(rel_rot);

            signals_data.exit_signals.push_back(signal);

            Journal::instance()->info("Loaded exit signal " + signal->getLetter());
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
            signal->setLetter(signal_letter);
            signal->setDirection(signal_dir_bwd);
            signal->setSignalModel(signal_model_bwd);
            signal->setConnector(conn);

            conn->setSignalBwd(signal);

            signal->setRelPosition(rel_pos);
            signal->setRelRotation(rel_rot);

            signals_data.line_signals.push_back(signal);

            Journal::instance()->info("Loaded line signal " + signal->getLetter());
        }

        if ( (signal_model_bwd.right(4) == "entr") || (signal_model_bwd.right(4) == "rout") )
        {
            EnterSignal *signal = new EnterSignal;
            signal->setLetter(signal_letter);
            signal->setDirection(signal_dir_bwd);
            signal->setSignalModel(signal_model_bwd);
            signal->setConnector(conn);

            conn->setSignalBwd(signal);

            signal->setRelPosition(rel_pos);
            signal->setRelRotation(rel_rot);

            signals_data.enter_signals.push_back(signal);

            Journal::instance()->info("Loaded enter signal " + signal->getLetter());
        }

        if (signal_model_bwd.right(4) == "exit")
        {
            ExitSignal *signal = new ExitSignal;
            signal->setLetter(signal_letter);
            signal->setDirection(signal_dir_bwd);
            signal->setSignalModel(signal_model_bwd);
            signal->setConnector(conn);

            conn->setSignalBwd(signal);

            signal->setRelPosition(rel_pos);
            signal->setRelRotation(rel_rot);

            signals_data.exit_signals.push_back(signal);

            Journal::instance()->info("Loaded exit signal " + signal->getLetter());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::line_signals_connect(std::vector<Signal *> &line_signals)
{
    for (auto signal : line_signals)
    {
        Connector *conn = signal->getConnector();

        if (conn == nullptr)
        {
            Journal::instance()->error("Failed connector for signal " + signal->getLetter());
            continue;
        }

        bool is_not_found = true;

        while (is_not_found)
        {
            Trajectory *traj = nullptr;

            if (signal->getDirection() == 1)
            {
                traj = conn->getBwdTraj();
            }

            if (signal->getDirection() == -1)
            {
                traj = conn->getFwdTraj();
            }

            if (traj == nullptr)
            {
                is_not_found = false;
                continue;
            }

            if (signal->getDirection() == 1)
            {
                conn = traj->getBwdConnector();
            }

            if (signal->getDirection() == -1)
            {
                conn = traj->getFwdConnector();
            }

            if (conn == nullptr)
            {
                is_not_found = false;
                continue;
            }

            Signal *prev_signal = nullptr;

            if (signal->getDirection() == 1)
            {
                prev_signal = conn->getSignalFwd();
            }

            if (signal->getDirection() == -1)
            {
                prev_signal = conn->getSignalBwd();
            }

            if (prev_signal != nullptr)
            {
                connect(signal, &Signal::sendLineVoltage,
                        prev_signal, &Signal::slotRecvLineVoltage);

                is_not_found = false;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::enter_signal_connect(std::vector<Signal *> &enter_signals)
{
    for (auto signal : enter_signals)
    {
        Connector *conn = signal->getConnector();

        if (conn == nullptr)
        {
            Journal::instance()->error("Failed connector for signal " + signal->getLetter());
            continue;
        }

        bool is_not_found = true;

        while (is_not_found)
        {
            Trajectory *traj = nullptr;

            if (signal->getDirection() == 1)
            {
                traj = conn->getBwdTraj();
            }

            if (signal->getDirection() == -1)
            {
                traj = conn->getFwdTraj();
            }

            if (traj == nullptr)
            {
                is_not_found = false;
                continue;
            }

            if (signal->getDirection() == 1)
            {
                conn = traj->getBwdConnector();
            }

            if (signal->getDirection() == -1)
            {
                conn = traj->getFwdConnector();
            }

            if (conn == nullptr)
            {
                is_not_found = false;
                continue;
            }

            Signal *prev_signal = nullptr;

            if (signal->getDirection() == 1)
            {
                prev_signal = conn->getSignalFwd();
            }

            if (signal->getDirection() == -1)
            {
                prev_signal = conn->getSignalBwd();
            }

            if (prev_signal != nullptr)
            {
                connect(signal, &Signal::sendLineVoltage,
                        prev_signal, &Signal::slotRecvLineVoltage);

                connect(signal, &Signal::sendSideVoltage,
                        prev_signal, &Signal::slotRecvSideVoltage);

                is_not_found = false;
            }
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
void Topology::slotSetSwitchState(QByteArray &switch_data)
{
    switch_state_t sw_state;
    sw_state.deserialize(switch_data);

    Switch *sw = dynamic_cast<Switch *>(switches.value(sw_state.name, nullptr));

    if (sw == nullptr)
    {
        return;
    }

    sw->setRefStateFwd(sw_state.state_fwd);
    sw->setRefStateBwd(sw_state.state_bwd);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotGetSwitchState(QByteArray &switch_data)
{
    switch_state_t sw_state;
    sw_state.deserialize(switch_data);

    Switch *sw = dynamic_cast<Switch *>(switches.value(sw_state.name, nullptr));

    if (sw == nullptr)
    {
        return;
    }

    sw_state.state_fwd = sw->getStateFwd();
    sw_state.state_bwd = sw->getStateBwd();

    switch_data = sw_state.serialize();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotOpenSignal(QByteArray signal_data)
{
    Journal::instance()->info("Try to open signal...");

    QBuffer buff(&signal_data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    QString conn_name = "";
    int sig_dir = 0;
    stream >> conn_name;
    stream >> sig_dir;

    if (conn_name.isEmpty())
    {
        Journal::instance()->error("Connector name is empty");
        return;
    }

    if (sig_dir == 0)
    {
        Journal::instance()->error("Signal direction is zero");
        return;
    }

    Connector *conn = switches.value(conn_name, nullptr);

    if (conn == nullptr)
    {
        return;
    }

    Signal *signal = nullptr;

    if (sig_dir == 1)
    {
        signal = conn->getSignalFwd();
    }

    if (sig_dir == -1)
    {
        signal = conn->getSignalBwd();
    }

    if (signal == nullptr)
    {
        return;
    }

    if ( (signal->getSignalType() == "entr") || (signal->getSignalType() == "rout") )
    {
        EnterSignal *es = dynamic_cast<EnterSignal *>(signal);

        if (es == nullptr)
        {
            return;
        }

        es->slotPressOpen();
    }

    if (signal->getSignalType() == "exit")
    {
        ExitSignal *es = dynamic_cast<ExitSignal *>(signal);

        if (es == nullptr)
        {
            return;
        }

        es->slotPressOpen();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotCloseSignal(QByteArray signal_data)
{
    Journal::instance()->info("Try to close signal...");

    QBuffer buff(&signal_data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    QString conn_name = "";
    int sig_dir = 0;
    stream >> conn_name;
    stream >> sig_dir;

    if (conn_name.isEmpty())
    {
        Journal::instance()->error("Connector name is empty");
        return;
    }

    if (sig_dir == 0)
    {
        Journal::instance()->error("Signal direction is zero");
        return;
    }

    Connector *conn = switches.value(conn_name, nullptr);

    if (conn == nullptr)
    {
        return;
    }

    Signal *signal = nullptr;

    if (sig_dir == 1)
    {
        signal = conn->getSignalFwd();
    }

    if (sig_dir == -1)
    {
        signal = conn->getSignalBwd();
    }

    if (signal == nullptr)
    {
        return;
    }

    if ( (signal->getSignalType() == "entr") || (signal->getSignalType() == "rout") )
    {
        EnterSignal *es = dynamic_cast<EnterSignal *>(signal);

        if (es == nullptr)
        {
            return;
        }

        es->slotPressClose();
    }

    if (signal->getSignalType() == "exit")
    {
        ExitSignal *es = dynamic_cast<ExitSignal *>(signal);

        if (es == nullptr)
        {
            return;
        }

        es->slotPressClose();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::slotBuildRoute(QString start_traj, QString target_traj, int dir)
{
    auto s_traj = traj_list.value(start_traj, nullptr);

    if (s_traj == nullptr)
    {
        Journal::instance()->error("BuildRoute: Unknown start trajectory " + start_traj);
        return;
    }

    auto t_traj = traj_list.value(target_traj, nullptr);

    if (t_traj == nullptr)
    {
        Journal::instance()->error("BuildRoute: Unknown target trajectory " + target_traj);
        return;
    }

    auto route_opt = find_route(s_traj, t_traj, dir);

    if (!route_opt)
    {
        Journal::instance()->error("Build route: NO topological route from " + start_traj + " to " + target_traj);
        return;
    }

    auto &route = route_opt.value();

    if (route.empty())
    {
        Journal::instance()->error("Build route: EMPTY route from " + start_traj + " to " + target_traj);
        return;
    }
    Journal::instance()->info("Build route: founded from " + start_traj + " to " + target_traj + " through " + QString::number(route.size()) + "trajectories");

    if (!set_switchs_by_route(route, dir))
    {
        Journal::instance()->error("Build route: Route is occupied or switches cannot be set");
        return;
    }

    QStringList signals_for_open;

    if (!open_route_signals(route, dir, signals_for_open))
    {
        Journal::instance()->error("Build route: Can't open route signals");
    }

    emit sigSetOpenSignalsQueue(signals_for_open, dir);
}
