#ifndef     ROUTE_SEGMENT_H
#define     ROUTE_SEGMENT_H

#include    <trajectory.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_segment_t
{
    Trajectory *traj = nullptr;
    Connector *next_conn = nullptr;
    int dir = 1;

    route_segment_t()
    {

    }
};

#endif
