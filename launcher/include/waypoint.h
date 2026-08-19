#ifndef     WAYPOINT_H
#define     WAYPOINT_H

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct train_position_t
{
    QString     name = "";
    QString     trajectory_name = "";
    int         direction = 1;
    double      traj_coord = 0.0;
    double      railway_coord = 0.0;
};

#endif // WAYPOINT_H
