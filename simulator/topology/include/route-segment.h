#ifndef     ROUTE_SEGMENT_H
#define     ROUTE_SEGMENT_H

#include    <trajectory.h>

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
