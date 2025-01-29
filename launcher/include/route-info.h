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

#include    <QString>
class TrainWaypointWidget;

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
    /// Saved last trains and its start waypoints
    std::vector<TrainWaypointWidget *> last_train_waypoints = {};

    route_info_t()
    {

    }
};

#endif // ROUTEINFO_H
