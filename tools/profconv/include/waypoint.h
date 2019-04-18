#ifndef     WAYPOINT_H
#define     WAYPOINT_H

#include    <string>

struct waypoint_t
{
    std::wstring    name;
    size_t          forward_track;
    size_t          backward_track;
};

#endif // WAYPOINT_H
