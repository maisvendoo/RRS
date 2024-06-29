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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_info_t
{
    /// Route directory path
    QString route_dir_full_path;
    /// Route directory name
    QString route_dir_name;
    /// Route name
    QString route_title;
    /// Route description
    QString route_description;

    route_info_t()
        : route_dir_full_path("")
        , route_dir_name("")
        , route_title("")
        , route_description("")
    {

    }
};

#endif // ROUTEINFO_H
