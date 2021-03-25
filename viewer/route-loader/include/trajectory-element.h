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

#include    <array>

#include    "global-const.h"
#include    "vehicle-signals.h"
#include    "server-data-struct.h"

#include    <QString>

#include    <deque>

/*!
 * \struct
 * \brief Trajectory element
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct network_data_t
{
    float                       delta_time;
    std::deque<server_data_t>   sd;

    network_data_t()
        : delta_time(0.1f)
    {

    }
};

#endif // TRAJECTORY_ELEMENT_H
