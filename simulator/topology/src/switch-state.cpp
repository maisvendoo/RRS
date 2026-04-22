#include "switch-state.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

QByteArray switch_command_t::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << conn_name;
    stream << switch_direction;
    stream << switch_ref_state;

    return data;
}

void switch_command_t::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> conn_name;
    stream >> switch_direction;
    stream >> switch_ref_state;
}

QByteArray switch_state_t::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << name;
    stream << state_fwd;
    stream << state_bwd;

    return data;
}

void switch_state_t::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> name;
    stream >> state_fwd;
    stream >> state_bwd;
}
