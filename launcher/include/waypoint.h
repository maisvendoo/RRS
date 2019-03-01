#ifndef     WAYPOINT_H
#define     WAYPOINT_H

#include    <QString>

struct waypoint_t
{
    QString     name;
    double      railway_coord;

    waypoint_t()
        : name("")
        , railway_coord(0.0)
    {

    }
};

#endif // WAYPOINT_H
