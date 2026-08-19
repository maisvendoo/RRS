#ifndef     ROUTE_COMMAND_H
#define     ROUTE_COMMAND_H

#include    "topology-export.h"

#include    <QByteArray>
#include    <QString>

#include    <cstdint>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct TOPOLOGY_EXPORT route_command_t
{
    /// Имя траектории, с которой начать поиск маршрута
    QString trajectory_begin = "";
    /// Имя траектории, до которой нужен маршрут (включительно)
    QString trajectory_end = "";
    /// Направление маршрута по топологии: 1 - вперёд, -1 - назад
    std::int8_t dir = 0;

    QByteArray serialize() const;
    void deserialize(QByteArray& data);
};

#endif // ROUTE_COMMAND_H
