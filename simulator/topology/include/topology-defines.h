#ifndef TOPOLOGY_DEFINES_H
#define TOPOLOGY_DEFINES_H

#include    <cstdint>
#include    <QString>
#include    <QMap>

class Trajectory;
class Switch;

/// Список траекторий по именам
using traj_list_t = QMap<QString, Trajectory *>;

/// Список соединений между траекториями по именам
using sw_list_t = QMap<QString, Switch *>;

/// Направление прохода по топологии
enum dir_t : std::int8_t
{
    FWD = 1,    ///< Движение по топологии совпадает с ориентацией траектории/стрелки
    BWD = -1,   ///< Движение по топологии противоположно ориентации траектории/стрелки
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum Switch_way_t : std::int8_t
{
    SW_FWD_PLUS = 0,///< Направление от стрелки спереди прямо
    SW_FWD_MINUS,   ///< Направление от стрелки спереди на отклонение
    SW_BWD_PLUS,    ///< Направление от стрелки сзади прямо
    SW_BWD_MINUS,   ///< Направление от стрелки сзади на отклонение
};
constexpr std::initializer_list<Switch_way_t> switch_ways_t = {SW_FWD_PLUS, SW_FWD_MINUS, SW_BWD_PLUS, SW_BWD_MINUS};
constexpr std::initializer_list<Switch_way_t> switch_fwd_ways_t = {SW_FWD_PLUS, SW_FWD_MINUS};
constexpr std::initializer_list<Switch_way_t> switch_bwd_ways_t = {SW_BWD_PLUS, SW_BWD_MINUS};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum Switch_state_t : std::int8_t
{
    ONLY_MINUS = -1,        ///< Единственная траектория в минусовом направлении
    ONLY_PLUS = 1,          ///< Единственная траектория в плюсовом направлении
    STATE_MINUS = -2,       ///< Стрелка в минусовом положении (на отклонение)
    STATE_PLUS = 2,         ///< Стрелка в плюсовом положении (прямо)
    IS_BUSY_MINUS = -3,     ///< Стрелка занята ПЕ в минусовом положении
    IS_BUSY_PLUS = 3,       ///< Стрелка занята ПЕ в плюсовом положении
    IN_ROUTE_MINUS = -4,    ///< Стрелка в маршруте в минусовом положении
    IN_ROUTE_PLUS = 4,      ///< Стрелка в маршруте в плюсовом положении
    NO_POSSIBLE_DIRECTION = 0   ///< Нет возможных траекторий
};

#endif // TOPOLOGY_DEFINES_H
