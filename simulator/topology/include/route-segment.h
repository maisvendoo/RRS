#ifndef     ROUTE_SEGMENT_H
#define     ROUTE_SEGMENT_H

#include    <QString>
#include    <QBuffer>

class Trajectory;
//#include    <trajectory.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_command_t
{
    QString trajectory_begin = "";  ///< Имя траектории, с которой начать поиск маршрута
    QString trajectory_end = "";    ///< Имя траектории, до которой нужен маршрут (включительно)
    std::int8_t dir = 0;    ///< Направление маршрута по топологии: 1 - вперёд, -1 - назад

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << trajectory_begin;
        stream << trajectory_end;
        stream << dir;

        return buff.data();
    }

    void deserialize(QByteArray data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> trajectory_begin;
        stream >> trajectory_end;
        stream >> dir;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_segment_t
{
    std::vector<Trajectory*> trajectories; ///< Траектории, входящие в маршрут
    int dir = 0; ///< Направление маршрута по топологии: 1 - вперёд, -1 - назад

    route_segment_t()
    {

    }
};

#endif
