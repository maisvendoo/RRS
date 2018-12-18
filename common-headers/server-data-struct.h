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

    server_data_t()
        : count(0)        
    {

    }
};

#pragma pack(pop)

#endif // SERVER_DATA_STRUCT_H
