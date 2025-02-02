//------------------------------------------------------------------------------
//
//      Info about route
//      (c) maisvendoo 17/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Info about route
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 17/12/2018
 */

#ifndef     ROUTEINFO_H
#define     ROUTEINFO_H

#include    <trajectory-info.h>
#include    <waypoint.h>

#include    "train-waypoint-widget.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_info_t
{
    /// Route directory path
    QString route_dir_full_path = "";
    /// Route directory name
    QString route_dir_name = "";
    /// Route name
    QString route_title = "";
    /// Route description
    QString route_description = "";

    /// Info about waypoints in route
    std::vector<train_position_t> fwd_train_positions;
    /// Info about waypoints in route
    std::vector<train_position_t> bwd_train_positions;
    /// Info about trajectories in route
    std::vector<trajectory_info_t>   trajectrories;

    /// Saved last trains and its start waypoints
    std::vector<TrainWaypointWidget *> last_train_waypoints = {};

    route_info_t()
    {

    }

    ~route_info_t()
    {
        for (auto ltw : last_train_waypoints)
            delete ltw;
    }
};

#endif // ROUTEINFO_H
