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

#endif // TOPOLOGY_TYPES_H
