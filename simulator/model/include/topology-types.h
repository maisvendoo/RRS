#ifndef     TOPOLOGY_TYPES_H
#define     TOPOLOGY_TYPES_H

#include    <QString>
#include    <QMap>

#include    "trajectory.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef     QMap<QString, Trajectory *> traj_list_t;

typedef     QMap<QString, Connector *> conn_list_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct topology_pos_t
{
    QString     traj_name;
    double      traj_coord;
    int         dir;

    topology_pos_t()
        : traj_name("")
        , traj_coord(0)
        , dir(1)
    {

    }
};

#endif // TOPOLOGY_TYPES_H
