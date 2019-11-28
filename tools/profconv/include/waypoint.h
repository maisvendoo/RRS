#ifndef     WAYPOINT_H
#define     WAYPOINT_H

#include    <QString>

struct waypoint_t
{
    QString         name;
    size_t          forward_track;
    size_t          backward_track;
};

#endif // WAYPOINT_H
