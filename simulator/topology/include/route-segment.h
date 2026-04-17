#ifndef     ROUTE_SEGMENT_H
#define     ROUTE_SEGMENT_H

#include <vector>
#include "topology-defines.h"

class Trajectory;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_segment_t
{
    std::vector<Trajectory*> trajectories; ///< Траектории, входящие в маршрут
    std::vector<dir_t> directions; ///< Направление по траекториям: 1 - вперёд, -1 - назад
};

#endif
