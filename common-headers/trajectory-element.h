//------------------------------------------------------------------------------
//
//      Current trajectory element for lineaer approximate of camera motion
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Current trajectory element for lineaer approximate of camera motion
 * \copyright maisvendoo
 * \author maisvendoo
 */

#ifndef     TRAJECTORY_ELEMENT_H
#define     TRAJECTORY_ELEMENT_H

#include    <osg/Referenced>
#include    <array>

#include    "global-const.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct traj_element_t
{
    float           coord_begin;
    float           coord_end;
    float           angle_begin;
    float           angle_end;

    traj_element_t()
        : coord_begin(0.0f)
        , coord_end(0.0f)
        , angle_begin(0.0f)
        , angle_end(0.0f)
    {

    }
};

/*!
 * \struct
 * \brief Trajectory element
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct network_data_t : public osg::Referenced
{
    unsigned int    route_id;
    float           delta_time;
    unsigned long   count;
    std::array<traj_element_t, MAX_NUM_VEHICLES>    te;

    network_data_t()
        : route_id(0)        
        , delta_time(3600.0f)
        , count(0)
    {

    }
};

#endif // TRAJECTORY_ELEMENT_H
