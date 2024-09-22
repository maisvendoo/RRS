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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Topology::Topology(QObject *parent) : QObject(parent)
{

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
bool Topology::load(QString route_dir)
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

    for (QString name : names)
    {
        Trajectory *traj = new Trajectory();

        if (traj->load(route_path, name))
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
bool Topology::init(const topology_pos_t &tp, std::vector<Vehicle *> *vehicles)
{
    if (vehicles->empty())
    {
        Journal::instance()->error("Vehicles list is empty!!!");
        return false;
    }

    vehicle_control.resize(vehicles->size());

    // Находим уазатель на стартовую траекторию
    Trajectory *cur_traj = traj_list.value(tp.traj_name, Q_NULLPTR);
    if (cur_traj == Q_NULLPTR)
    {
        Journal::instance()->critical("INVALID INITIAL TRAJECTORY!!!");
        return false;
    }

    double traj_coord = cut(tp.traj_coord, 0.0, cur_traj->getLength());

    for (size_t i = 0; i < vehicles->size(); ++i)
    {
        vehicle_control[i] = new VehicleController;

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
            if (conn == Q_NULLPTR)
            {
                Journal::instance()->error("Trajectory " + cur_traj->getName() + " has't forward connector");
                return false;
            }

            // Получаем указатель на следующую траекторию спереди
            Trajectory *next_traj = conn->getFwdTraj();
            if (next_traj == Q_NULLPTR)
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
            if (conn == Q_NULLPTR)
            {
                Journal::instance()->error("Trajectory " + cur_traj->getName() + " has't backward connector");
                return false;
            }

            // Получаем указатель на следующую траекторию сзади
            Trajectory *next_traj = conn->getBwdTraj();
            if (next_traj == Q_NULLPTR)
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

        vehicle_control[i]->setIndex(i);
        vehicle_control[i]->setLength(L);
        vehicle_control[i]->setVehicleRailwayConnectors((*vehicles)[i]->getRailwayConnectors());
        vehicle_control[i]->setInitCurrentTraj(cur_traj, traj_coord);
        vehicle_control[i]->setDirection(tp.dir);
        vehicle_control[i]->setInitCoord((*vehicles)[i]->getTrainCoord());

        Journal::instance()->info(QString("Vehcile #%1").arg(i) +
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

            if (conn == Q_NULLPTR)
            {
                continue;

            }

            Trajectory *traj = conn->getBwdTraj();

            if (traj == Q_NULLPTR)
            {
                continue;
            }

            is_busy = traj->isBusy();
        }

        if (line_signal->getDirection() == -1)
        {
            Connector *conn = line_signal->getConnector();

            if (conn == Q_NULLPTR)
            {
                continue;
            }

            Trajectory *traj = conn->getFwdTraj();

            if (traj == Q_NULLPTR)
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

        if (es == Q_NULLPTR)
        {
            continue;
        }

        if (es->getDirection() == 1)
        {
            Connector *conn = es->getConnector();

            if (conn == Q_NULLPTR)
            {
                continue;

            }

            Trajectory *bwd_traj = conn->getBwdTraj();
            Trajectory *fwd_traj = conn->getFwdTraj();

            if ( (bwd_traj == Q_NULLPTR) || (fwd_traj == Q_NULLPTR) )
            {
                continue;
            }

            es->setFwdBusy(fwd_traj->isBusy());
            es->setBwdBusy(bwd_traj->isBusy());
        }

        if (es->getDirection() == -1)
        {
            Connector *conn = es->getConnector();

            if (conn == Q_NULLPTR)
            {
                continue;

            }

            Trajectory *bwd_traj = conn->getFwdTraj();
            Trajectory *fwd_traj = conn->getBwdTraj();

            if ( (bwd_traj == Q_NULLPTR) || (fwd_traj == Q_NULLPTR) )
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
        if (signal == Q_NULLPTR)
        {
            continue;
        }

        signal->step(t, dt);
    }
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

    stream << stations.size();

    for (auto station : stations)
    {
        QByteArray sdata = station.serialize();
        stream << sdata;
    }

    // Указываем число коннекторов
    stream << switches.size();

    // Складываем в буффер сериализованную информацию о коннекторах
    for (auto sw = switches.begin(); sw != switches.end(); ++sw)
    {
        stream << sw.value()->serialize();
    }

    stream << traj_list.size();

    for (auto traj = traj_list.begin(); traj != traj_list.end(); ++traj)
    {
        stream << traj.value()->serialize();
        serialize_connector_name(stream, traj.value()->getFwdConnector());
        serialize_connector_name(stream, traj.value()->getBwdConnector());
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

    qsizetype stations_count = 0;
    stream >> stations_count;

    stations.clear();

    for (qsizetype i = 0; i < stations_count; ++i)
    {
        QByteArray station_data;
        stream >> station_data;
        topology_station_t station;
        station.deserialize(station_data);
        stations.push_back(station);
    }

    traj_list.clear();
    switches.clear();

    qsizetype conn_count = 0;
    stream >> conn_count;

    for (qsizetype i = 0; i < conn_count; ++i)
    {
        Switch *sw = new Switch;

        QByteArray conn_data;
        stream >> conn_data;

        sw->deserialize(conn_data, traj_list);

        switches.insert(sw->getName(), sw);
    }

    qsizetype traj_count = 0;
    stream >> traj_count;

    for (auto traj : traj_list)
    {
        QByteArray traj_data;
        stream >> traj_data;
        traj->deserialize(traj_data);
        traj->setFwdConnector(deserialize_traj_connectors(stream, switches));
        traj->setBwdConnector(deserialize_traj_connectors(stream, switches));
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
void Topology::load_signals(CfgReader &cfg, QDomNode secNode, Connector *conn)
{
    QString signal_model = "";
    cfg.getString(secNode, "SignalModel", signal_model);

    if (!signal_model.isEmpty())
    {
        QString signal_letter = "";
        cfg.getString(secNode, "SignalLetter", signal_letter);
        int signal_dir = 0;
        cfg.getInt(secNode, "SignalDirection", signal_dir);

        if (signal_model.right(4) == "line")
        {
            LineSignal *line_signal = new LineSignal;
            line_signal->setLetter(signal_letter);
            line_signal->setDirection(signal_dir);
            line_signal->setSignalModel(signal_model);
            line_signal->setConnector(conn);

            conn->setSignal(line_signal);

            signals_data.line_signals.push_back(line_signal);

            Journal::instance()->info("Loaded line signal " + line_signal->getLetter());
        }

        if (signal_model.right(4) == "entr")
        {
            EnterSignal *enter_signal = new EnterSignal;
            enter_signal->setLetter(signal_letter);
            enter_signal->setDirection(signal_dir);
            enter_signal->setSignalModel(signal_model);
            enter_signal->setConnector(conn);

            conn->setSignal(enter_signal);

            signals_data.enter_signals.push_back(enter_signal);

            Journal::instance()->info("Loaded enter signal " + enter_signal->getLetter());
        }

        if (signal_model.right(4) == "exit")
        {
            ExitSignal *signal = new ExitSignal;
            signal->setLetter(signal_letter);
            signal->setDirection(signal_dir);
            signal->setSignalModel(signal_model);
            signal->setConnector(conn);

            conn->setSignal(signal);

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
    for (auto line_signal : line_signals)
    {
        Connector *conn = line_signal->getConnector();

        if (conn == Q_NULLPTR)
        {
            continue;
        }

        if (line_signal->getDirection() == 1)
        {
            Trajectory *traj = conn->getBwdTraj();
            if (traj == Q_NULLPTR)
            {
                continue;
            }

            Connector *conn = traj->getBwdConnector();

            if (conn == Q_NULLPTR)
            {
                continue;
            }

            Signal *line_signal_prev = conn->getSignal();

            if (line_signal_prev == Q_NULLPTR)
            {
                continue;
            }

            connect(line_signal, &Signal::sendLineVoltage,
                    line_signal_prev, &Signal::slotRecvLineVoltage);

            Journal::instance()->info("Connected line signal " + line_signal->getLetter() +
                                      " with line signal " + line_signal_prev->getLetter());
        }

        if (line_signal->getDirection() == -1)
        {
            Trajectory *traj = conn->getFwdTraj();
            if (traj == Q_NULLPTR)
            {
                continue;
            }

            Connector *conn = traj->getFwdConnector();

            if (conn == Q_NULLPTR)
            {
                continue;
            }

            Signal *line_signal_prev = conn->getSignal();

            if (line_signal_prev == Q_NULLPTR)
            {
                continue;
            }

            connect(line_signal, &Signal::sendLineVoltage,
                    line_signal_prev, &Signal::slotRecvLineVoltage);

            Journal::instance()->info("Connected line signal " + line_signal->getLetter() +
                                      " with line signal " + line_signal_prev->getLetter());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::enter_signal_connect(std::vector<Signal *> &enter_signals)
{
    for (auto enter_signal : enter_signals)
    {
        Connector *conn = enter_signal->getConnector();

        if (conn == Q_NULLPTR)
        {
            continue;
        }

        if (enter_signal->getDirection() == 1)
        {
            Trajectory *traj = conn->getBwdTraj();
            if (traj == Q_NULLPTR)
            {
                continue;
            }

            Connector *conn = traj->getBwdConnector();

            if (conn == Q_NULLPTR)
            {
                continue;
            }

            Signal *line_signal_prev = conn->getSignal();

            if (line_signal_prev == Q_NULLPTR)
            {
                continue;
            }

            connect(enter_signal, &Signal::sendLineVoltage,
                    line_signal_prev, &Signal::slotRecvLineVoltage);

            connect(enter_signal, &Signal::sendSideVoltage,
                    line_signal_prev, &Signal::slotRecvSideVoltage);

            Journal::instance()->info("Connected line signal " + enter_signal->getLetter() +
                                      " with line signal " + line_signal_prev->getLetter());
        }

        if (enter_signal->getDirection() == -1)
        {
            Trajectory *traj = conn->getFwdTraj();
            if (traj == Q_NULLPTR)
            {
                continue;
            }

            Connector *conn = traj->getFwdConnector();

            if (conn == Q_NULLPTR)
            {
                continue;
            }

            Signal *line_signal_prev = conn->getSignal();

            if (line_signal_prev == Q_NULLPTR)
            {
                continue;
            }

            connect(enter_signal, &Signal::sendLineVoltage,
                    line_signal_prev, &Signal::slotRecvLineVoltage);

            connect(enter_signal, &Signal::sendSideVoltage,
                    line_signal_prev, &Signal::slotRecvSideVoltage);

            Journal::instance()->info("Connected line signal " + enter_signal->getLetter() +
                                      " with line signal " + line_signal_prev->getLetter());
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
    if (bool has_conn = conn != Q_NULLPTR)
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

        return conn_list.value(conn_name, Q_NULLPTR);
    }

    return Q_NULLPTR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Topology::getSwitchState(QByteArray &switch_data)
{
    switch_state_t sw_state;
    sw_state.deserialize(switch_data);

    Switch *sw = dynamic_cast<Switch *>(switches.value(sw_state.name, Q_NULLPTR));

    if (sw == Q_NULLPTR)
    {
        return;
    }

    sw->setRefStateFwd(sw_state.state_fwd);
    sw->setRefStateBwd(sw_state.state_bwd);
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
    stream >> conn_name;

    if (conn_name.isEmpty())
    {
        return;
    }

    Connector *conn = switches.value(conn_name, Q_NULLPTR);

    if (conn == Q_NULLPTR)
    {
        return;
    }

    Signal *signal = conn->getSignal();

    if (signal == Q_NULLPTR)
    {
        return;
    }

    if (signal->getSignalType() == "entr")
    {
        EnterSignal *es = dynamic_cast<EnterSignal *>(signal);

        if (es == Q_NULLPTR)
        {
            return;
        }

        es->slotPressOpen();
    }

    if (signal->getSignalType() == "exit")
    {
        ExitSignal *es = dynamic_cast<ExitSignal *>(signal);

        if (es == Q_NULLPTR)
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
    stream >> conn_name;

    if (conn_name.isEmpty())
    {
        return;
    }

    Connector *conn = switches.value(conn_name, Q_NULLPTR);

    if (conn == Q_NULLPTR)
    {
        return;
    }

    Signal *signal = conn->getSignal();

    if (signal == Q_NULLPTR)
    {
        return;
    }

    if (signal->getSignalType() == "entr")
    {
        EnterSignal *es = dynamic_cast<EnterSignal *>(signal);

        if (es == Q_NULLPTR)
        {
            return;
        }

        es->slotPressClose();
    }

    if (signal->getSignalType() == "exit")
    {
        ExitSignal *es = dynamic_cast<ExitSignal *>(signal);

        if (es == Q_NULLPTR)
        {
            return;
        }

        es->slotPressClose();
    }
}
