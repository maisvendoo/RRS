#ifndef     SIGNAL_COMMAND_H
#define     SIGNAL_COMMAND_H

#include    <QBuffer>

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

    QByteArray serialize()
    {
        QByteArray tmp_data;
        QBuffer buff(&tmp_data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << conn_name;
        stream << sig_dir;
        stream << command_open_train;
        stream << command_open_shunting;
        stream << command_open_call;
        stream << command_close;

        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> conn_name;
        stream >> sig_dir;
        stream >> command_open_train;
        stream >> command_open_shunting;
        stream >> command_open_call;
        stream >> command_close;
    }
};

#endif
