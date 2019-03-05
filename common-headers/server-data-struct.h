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

#include    <QString>

enum
{
    MAX_DISCRETE_SIGNALS = 1000,
    MAX_ANALOG_SIGNALS = 1000
};

struct vehicle_data_t
{
    float   railway_coord;
    float   velocity;
    float   wheel_angle;
    float   omega;
    wchar_t DebugMsg[1024];
    bool    discreteSignal[MAX_DISCRETE_SIGNALS];
    float   analogSignal[MAX_ANALOG_SIGNALS];

    vehicle_data_t()
        : railway_coord(0.0f)
        , velocity(0.0f)
        , wheel_angle(0.0f)
        , omega(0.0f)
        , DebugMsg(L"")
    {
        memset(discreteSignal, 0, sizeof (bool) * MAX_DISCRETE_SIGNALS);
        memset(analogSignal, 0, sizeof (float) * MAX_ANALOG_SIGNALS);
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
