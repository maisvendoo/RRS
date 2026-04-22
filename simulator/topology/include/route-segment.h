#ifndef     ROUTE_SEGMENT_H
#define     ROUTE_SEGMENT_H

#include    "topology-defines.h"

#include    <vector>

class Trajectory;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_segment_t
{
    /// Траектории, входящие в маршрут
    std::vector<Trajectory*> trajectories;
    /// Направление по траекториям: 1 - вперёд, -1 - назад
    std::vector<dir_t> directions;
};

#endif // ROUTE_SEGMENT_H
