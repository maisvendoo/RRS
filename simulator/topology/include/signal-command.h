#ifndef     SIGNAL_COMMAND_H
#define     SIGNAL_COMMAND_H

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>
#include    <QString>

#include    <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct signal_command_t
{
    QString conn_name = "";
    std::int8_t sig_dir = 0;
    bool command_open_train = false;
    bool command_open_shunting = false;
    bool command_open_call = false;
    bool command_close = false;

    QByteArray serialize() const
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

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> conn_name;
        stream >> sig_dir;
        stream >> command_open_train;
        stream >> command_open_shunting;
        stream >> command_open_call;
        stream >> command_close;
    }
};

#endif // SIGNAL_COMMAND_H
