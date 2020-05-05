#include    "topology.h"

#include    <QDir>
#include    <QDirIterator>

#include    "CfgReader.h"

#include    "switch.h"
#include    "isolated-joint.h"

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
    QStringList names = getTrajNamesList(route_dir);

    if (names.isEmpty())
        return false;

    for (QString name : names)
    {
        Trajectory *traj = new Trajectory();
        traj->load(route_dir, name);

        traj_list.insert(name, traj);
    }

    if (traj_list.size() == 0)
        return false;

    load_topology(route_dir);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Topology::init(const topology_pos_t &tp, std::vector<Vehicle *> *vehicles)
{
    vehicle_control.resize(vehicles->size());

    vehicle_control[0]->setTrajCoord(tp.traj_coord);
    vehicle_control[0]->setCurrentTraj(traj_list[tp.traj_name]);
    vehicle_control[0]->setDirection(tp.dir);
    vehicle_control[0]->setInitRailwayCoord((*vehicles)[0]->getRailwayCoord());

    double traj_coord = tp.traj_coord;
    Trajectory *cur_traj = traj_list[tp.traj_name];

    for (size_t i = 0; i < vehicles->size(); ++i)
    {
        double L = (vehicles->at(i - 1)->getLength() + vehicles->at(i)->getLength()) / 2.0;

        double new_traj_coord = traj_coord - tp.dir * L;

        if (new_traj_coord < 0)
        {
            traj_coord = cur_traj->getLength() + new_traj_coord;
            cur_traj = cur_traj->getBwdConnector()->getBwdTraj();
        }

        if (new_traj_coord > cur_traj->getLength())
        {
            traj_coord = new_traj_coord - cur_traj->getLength();
            cur_traj = cur_traj->getFwdConnector()->getFwdTraj();
        }

        if (cur_traj == Q_NULLPTR)
        {
            return false;
        }

        vehicle_control[i]->setTrajCoord(traj_coord);
        vehicle_control[i]->setCurrentTraj(cur_traj);
        vehicle_control[i]->setDirection(tp.dir);
        vehicle_control[i]->setInitRailwayCoord((*vehicles)[i]->getRailwayCoord());
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleController *Topology::getVehicleController(size_t idx) const
{
    return vehicle_control[idx];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QStringList Topology::getTrajNamesList(QString route_dir)
{
    QDir traj_dir(route_dir + QDir::separator() + "trajectories");

    QDirIterator traj_files(traj_dir.path(),
                            QStringList() << "*.trk",
                            QDir::NoDotAndDotDot | QDir::Files);

    QStringList names_list;

    while (traj_files.hasNext())
    {
        QString fullpath = traj_files.next();
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
    QString path = route_dir + QDir::separator() + "topology.xml";

    CfgReader cfg;

    if (!cfg.load(path))
        return false;

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
