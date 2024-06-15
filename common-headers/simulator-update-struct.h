#ifndef     SIMULATOR_UPDATE_STRUCT_H
#define     SIMULATOR_UPDATE_STRUCT_H

#include    "global-const.h"

#include    "vehicle-signals.h"

#include    <array>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_update_t
{
    double  position_x;
    double  position_y;
    double  position_z;
    double  orth_x;
    double  orth_y;
    double  orth_z;
    double  up_x;
    double  up_y;
    double  up_z;
    int     orientation;
    int     prev_vehicle;
    int     next_vehicle;
    std::array<float, MAX_ANALOG_SIGNALS>   analogSignal;

    simulator_vehicle_update_t()
        : position_x(0.0), position_y(0.0), position_z(0.0)
        , orth_x(0.0), orth_y(0.0), orth_z(0.0)
        , up_x(0.0), up_y(0.0), up_z(1.0)
        , orientation(1)
        , prev_vehicle(-1)
        , next_vehicle(-1)
    {
        std::fill(analogSignal.begin(), analogSignal.end(), 0.0f);
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_update_t
{
    double  time;
    int     current_vehicle;
    wchar_t currentDebugMsg[DEBUG_STRING_SIZE];
    int     controlled_vehicle;
    wchar_t controlledDebugMsg[DEBUG_STRING_SIZE];
    std::array<simulator_vehicle_update_t, MAX_NUM_VEHICLES>  vehicles;

    simulator_update_t()
        : time(0.0)
        , current_vehicle(0)
        , currentDebugMsg(L"")
        , controlled_vehicle(0)
        , controlledDebugMsg(L"")
    {

    }
};

#endif // SIMULATOR_UPDATE_STRUCT_H
