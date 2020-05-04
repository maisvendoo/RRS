#include    "topology.h"

#include    <QDir>
#include    <QDirIterator>

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
