#include "signal-command.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

QByteArray signal_command_t::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << conn_name;
    stream << sig_dir;
    stream << command_open_train;
    stream << command_open_shunting;
    stream << command_open_call;
    stream << command_close;

    return data;
}

void signal_command_t::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> conn_name;
    stream >> sig_dir;
    stream >> command_open_train;
    stream >> command_open_shunting;
    stream >> command_open_call;
    stream >> command_close;
}
