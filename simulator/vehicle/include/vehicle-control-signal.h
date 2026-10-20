#ifndef     VEHICLE_CONTROL_SIGNAL_H
#define     VEHICLE_CONTROL_SIGNAL_H

#include    <control-priority.h>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct vehicle_control_signal_t
{
    float value = 0.0f;

    std::vector<float> params;

    ControlPriority server_priority = CTRL_PRIORITY_FULL;

    vehicle_control_signal_t()
    {

    }

    bool toBool() const
    {
        return static_cast<bool>(value);
    }
};

#endif
