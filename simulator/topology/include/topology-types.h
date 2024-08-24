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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct topology_pos_t
{
    QString     traj_name = "";
    double      traj_coord = 0.0;
    int         dir = 1;

    topology_pos_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct topology_station_t
{
    QString name = "";
    double  pos_x = 0.0;
    double  pos_y = 0.0;
    double  pos_z = 0.0;

    topology_station_t()
    {

    }

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << name;
        stream << pos_x;
        stream << pos_y;
        stream << pos_z;

        return data;
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> name;
        stream >> pos_x;
        stream >> pos_y;
        stream >> pos_z;
    }
};

#endif
