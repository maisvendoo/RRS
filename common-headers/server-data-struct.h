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

const   int MAX_NUM_VEHICLES = 150;

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
    float               cabine_coord;
    float               railway_coords[MAX_NUM_VEHICLES];

    server_data_t()
        : count(0)
        , cabine_coord(0.0f)
    {

    }
};

#pragma pack(pop)

#endif // SERVER_DATA_STRUCT_H
