#include "topology-types.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

QByteArray topology_station_t::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << name;
    stream << pos_x;
    stream << pos_y;
    stream << pos_z;

    return data;
}

void topology_station_t::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> name;
    stream >> pos_x;
    stream >> pos_y;
    stream >> pos_z;
}

QByteArray traj_busy_state_t::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << name;
    stream << is_busy;
    stream << in_route;

    return data;
}

void traj_busy_state_t::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> name;
    stream >> is_busy;
    stream >> in_route;
}
