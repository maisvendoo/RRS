#ifndef     WAYPOINT_H
#define     WAYPOINT_H

#include    <string>

struct waypoint_t
{
    std::wstring    name;
    size_t          begin_track;
    size_t          end_track;
};

#endif // WAYPOINT_H
