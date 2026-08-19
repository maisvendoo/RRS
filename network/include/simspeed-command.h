#ifndef     SIMSPEED_COMMAND_H
#define     SIMSPEED_COMMAND_H

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simspeed_command_t
{
    int speed_factor = 1;

    QByteArray serialize()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << speed_factor;

        return data;
    }

    void deserialize(QByteArray data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> speed_factor;
    }
};

#endif // SIMSPEED_COMMAND_H
