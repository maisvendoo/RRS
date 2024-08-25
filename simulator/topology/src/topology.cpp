#include    <topology.h>

#include    <QDir>
#include    <QDirIterator>
#include    <QFile>

#include    <CfgReader.h>
#include    <switch.h>
#include    <isolated-joint.h>

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

    vehicle_control[0] = new VehicleController;
    vehicle_control[0]->setIndex(0);
    vehicle_control[0]->setVehicleRailwayConnectors((*vehicles)[0]->getRailwayConnectors());
    vehicle_control[0]->setTrajCoord(tp.traj_coord);
    vehicle_control[0]->setInitCurrentTraj(traj_list[tp.traj_name]);
    vehicle_control[0]->setDirection(tp.dir);
    vehicle_control[0]->setInitRailwayCoord((*vehicles)[0]->getRailwayCoord());

    double traj_coord = tp.traj_coord;

    Trajectory *cur_traj = traj_list.value(tp.traj_name, Q_NULLPTR);

    if (cur_traj == Q_NULLPTR)
    {
        Journal::instance()->critical("INVALID INITIAL TRAJECTORY!!!");
        return false;
    }

    Journal::instance()->info(QString("Vehcile #%1").arg(0) +
                              " at traj: " + cur_traj->getName() +
                              QString(" %1 m from start").arg(traj_coord));

    cur_traj->setBusy(true);

    for (size_t i = 1; i < vehicles->size(); ++i)
    {
        vehicle_control[i] = new VehicleController;

        double L = ((*vehicles)[i-1]->getLength() + (*vehicles)[i]->getLength()) / 2.0;

        traj_coord = traj_coord - tp.dir * L;

        while (traj_coord < 0)
        {
            Connector *conn = cur_traj->getBwdConnector();

            if (conn == Q_NULLPTR)
            {
                Journal::instance()->error("Trajectory " + cur_traj->getName() + " has't backward connector");
                return false;
            }

            cur_traj = conn->getBwdTraj();

            if (cur_traj == Q_NULLPTR)
            {
                Journal::instance()->error("Connector " + conn->getName() + " has't backward trajectory");
                return false;
            }

            traj_coord = cur_traj->getLength() + traj_coord;
        }

        while (traj_coord > cur_traj->getLength())
        {
            traj_coord = traj_coord - cur_traj->getLength();

            Connector *conn = cur_traj->getFwdConnector();

            if (conn == Q_NULLPTR)
            {
                Journal::instance()->error("Trajectory " + cur_traj->getName() + " has't forward connector");
                return false;
            }

            cur_traj = conn->getFwdTraj();

            if (cur_traj == Q_NULLPTR)
            {
                Journal::instance()->error("Connector " + conn->getName() + " has't forward trajectory");
                return false;
            }
        }

        vehicle_control[i]->setIndex(i);
        vehicle_control[i]->setVehicleRailwayConnectors((*vehicles)[i]->getRailwayConnectors());
        vehicle_control[i]->setTrajCoord(traj_coord);
        vehicle_control[i]->setInitCurrentTraj(cur_traj);
        vehicle_control[i]->setDirection(tp.dir);
        vehicle_control[i]->setInitRailwayCoord((*vehicles)[i]->getRailwayCoord());

        Journal::instance()->info(QString("Vehcile #%1").arg(i) +
                                  " at traj: " + cur_traj->getName() +
                                  QString(" %1 m from start").arg(traj_coord));

        cur_traj->setBusy(true);
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
void Topology::step(double t, double dt)
{
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

    sw->setStateFwd(sw_state.state_fwd);
    sw->setStateBwd(sw_state.state_bwd);

    switch_state_t new_state;
    new_state.name = sw->getName();
    new_state.state_fwd = sw->getStateFwd();
    new_state.state_bwd = sw->getStateBwd();

    emit sendSwitchState(new_state.serialize());
}
