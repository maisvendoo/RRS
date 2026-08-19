#ifndef     TRAFFIC_COMMON_TYPES_H
#define     TRAFFIC_COMMON_TYPES_H

#include    <QString>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct station_t
{
    QString     name;
    int         id;
    double      begin_coord;
    double      end_coord;

    station_t()
        : name("")
        , id(0)
        , begin_coord(0.0)
        , end_coord(0.0)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct waypoint_t
{
    int station_id;
    double dep_time;
    double arr_time;
    double ordinate;

    waypoint_t()
        : station_id(0)
        , dep_time(0)
        , arr_time(0)
        , ordinate(0)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using stations_list_t   = std::vector<station_t>;

#endif // TRAFFIC_COMMON_TYPES_H
