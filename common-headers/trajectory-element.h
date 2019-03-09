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
#include    "vehicle-signals.h"

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct traj_element_t
{
    float           coord_begin;
    float           coord_end;
    float           velocity;
    float           angle_begin;
    float           angle_end;
    float           omega;
    wchar_t         DebugMsg[DEBUG_SRING_SIZE];
    bool            discreteSignal[MAX_DISCRETE_SIGNALS];
    float           analogSignal[MAX_ANALOG_SIGNALS];

    traj_element_t()
        : coord_begin(0.0f)
        , coord_end(0.0f)
        , velocity(0.0f)
        , angle_begin(0.0f)
        , angle_end(0.0f)
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

#pragma pack(pop)

#endif // TRAJECTORY_ELEMENT_H
