#ifndef     TOPOLOGY_TYPES_H
#define     TOPOLOGY_TYPES_H

#include    <QString>
#include    <QBuffer>

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

    QByteArray serialize() const
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
    bool is_busy = false;
    bool in_route = false;

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << name;
        stream << is_busy;
        stream << in_route;

        return buff.data();
    }

    void deserialize(QByteArray data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> name;
        stream >> is_busy;
        stream >> in_route;
    }
};

#endif
