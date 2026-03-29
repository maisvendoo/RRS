#ifndef     SIMSPEED_COMMAND_H
#define     SIMSPEED_COMMAND_H

#include    <QBuffer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simspeed_command_t
{
    int speed_factor = 1;

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << speed_factor;

        return buff.data();
    }

    void deserialize(QByteArray data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> speed_factor;
    }
};

#endif
