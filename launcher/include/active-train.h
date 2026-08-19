#ifndef     ACTIVE_TRAIN_H
#define     ACTIVE_TRAIN_H

#include    <train-info.h>
#include    <waypoint.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct active_train_t
{
    bool is_active = false;
    train_info_t train_info;
    train_position_t train_position;
    bool is_autopilot_on = false;

    active_train_t()
    {

    }
};

#endif
