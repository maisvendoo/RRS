#ifndef     TOPOLOGY_TYPES_H
#define     TOPOLOGY_TYPES_H

#include    "topology-export.h"

#include    <QByteArray>
#include    <QString>
#include    <QVector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT topology_pos_t
{
    QString traj_name = "";
    double  traj_coord = 0.0;
    int     dir = 1;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT topology_station_t
{
    QString name = "";
    double  pos_x = 0.0;
    double  pos_y = 0.0;
    double  pos_z = 0.0;

    QByteArray serialize() const;
    void deserialize(QByteArray& data);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using topology_stations_list_t = QVector<topology_station_t>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT traj_busy_state_t
{
    QString name = "";
    bool    is_busy = false;
    bool    in_route = false;

    QByteArray serialize() const;
    void deserialize(QByteArray& data);
};

#endif // TOPOLOGY_TYPES_H
