#include "route-command.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

QByteArray route_command_t::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << trajectory_begin;
    stream << trajectory_end;
    stream << dir;

    return data;
}

void route_command_t::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> trajectory_begin;
    stream >> trajectory_end;
    stream >> dir;
}
