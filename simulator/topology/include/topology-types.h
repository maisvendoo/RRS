#ifndef     TOPOLOGY_TYPES_H
#define     TOPOLOGY_TYPES_H

#include    <QString>
#include    <QMap>

#include    <trajectory.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using traj_list_t = QMap<QString, Trajectory *>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using conn_list_t = QMap<QString, Connector *>;

#endif
