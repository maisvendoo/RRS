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
