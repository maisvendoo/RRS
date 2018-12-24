//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

#ifndef     SERVER_DATA_STRUCT_H
#define     SERVER_DATA_STRUCT_H

#include    <qglobal.h>

#include    "global-const.h"

#include    <array>

struct vehicle_data_t
{
    float   railway_coord;
    float   wheel_angle;

    vehicle_data_t()
        : railway_coord(0.0f)
        , wheel_angle(0.0f)
    {

    }
};

/*!
 * \struct
 * \brief Data structure, received from server
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#pragma pack(push, 1)

struct server_data_t
{
    /// Senden data count
    quint64             count;    
    std::array<vehicle_data_t, MAX_NUM_VEHICLES> vehicles_data;

    server_data_t()
        : count(0)        
    {

    }
};

#pragma pack(pop)

#endif // SERVER_DATA_STRUCT_H
