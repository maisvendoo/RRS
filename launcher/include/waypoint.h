#ifndef     WAYPOINT_H
#define     WAYPOINT_H

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct waypoint_t
{
    QString     name;
    double      forward_coord;
    double      backward_coord;

    waypoint_t()
        : name("")
        , forward_coord(0.0)
        , backward_coord(0.0)
    {

    }
};

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
