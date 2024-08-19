#include    <topology.h>

#include    <QDir>
#include    <QDirIterator>

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
QByteArray Topology::serialize()
{
    QBuffer data;
    data.open(QIODevice::WriteOnly);
    QDataStream stream(&data);

    stream << switches.size();

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

    int a = 0;
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
