#ifndef     ROUTE_COMMAND_H
#define     ROUTE_COMMAND_H

#include    <QByteArray>
#include    <QDataStream>
#include    <QIODevice>
#include    <QString>

#include    <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_command_t
{
    /// Имя траектории, с которой начать поиск маршрута
    QString trajectory_begin = "";
    /// Имя траектории, до которой нужен маршрут (включительно)
    QString trajectory_end = "";
    /// Направление маршрута по топологии: 1 - вперёд, -1 - назад
    std::int8_t dir = 0;

    QByteArray serialize()
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << trajectory_begin;
        stream << trajectory_end;
        stream << dir;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> trajectory_begin;
        stream >> trajectory_end;
        stream >> dir;
    }
};

#endif // ROUTE_COMMAND_H
