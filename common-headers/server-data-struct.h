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

#include    "vehicle-signals.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct vehicle_data_t
{
    float           coord;
    float           velocity;
    float           angle;
    float           omega;
    wchar_t         DebugMsg[DEBUG_STRING_SIZE];
    bool            discreteSignal[MAX_DISCRETE_SIGNALS];
    float           analogSignal[MAX_ANALOG_SIGNALS];

    vehicle_data_t()
        : coord(0.0f)
        , velocity(0.0f)
        , angle(0.0f)
        , omega(0.0f)
        , DebugMsg(L"")
    {
        memset(discreteSignal, 0, sizeof (bool) * MAX_DISCRETE_SIGNALS);
        memset(analogSignal, 0, sizeof (float) * MAX_ANALOG_SIGNALS);
    }
};



/*!
 * \struct
 * \brief Trajectory element
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#pragma pack(push, 1)

struct server_data_t
{
    unsigned int    route_id;
    float           time;
    unsigned long   count;

    std::array<vehicle_data_t, MAX_NUM_VEHICLES>    te;

    server_data_t()
        : route_id(0)
        , time(0.0f)
        , count(0)
    {

    }
};

#pragma pack(pop)

#endif // SERVER_DATA_STRUCT_H
