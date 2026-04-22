#ifndef     TOPOLOGY_TYPES_H
#define     TOPOLOGY_TYPES_H

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>
#include    <QString>
#include    <QVector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct topology_pos_t
{
    QString traj_name = "";
    double  traj_coord = 0.0;
    int     dir = 1;
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

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << name;
        stream << pos_x;
        stream << pos_y;
        stream << pos_z;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> name;
        stream >> pos_x;
        stream >> pos_y;
        stream >> pos_z;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using topology_stations_list_t = QVector<topology_station_t>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct traj_busy_state_t
{
    QString name = "";
    bool    is_busy = false;
    bool    in_route = false;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << name;
        stream << is_busy;
        stream << in_route;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> name;
        stream >> is_busy;
        stream >> in_route;
    }
};

#endif // TOPOLOGY_TYPES_H
