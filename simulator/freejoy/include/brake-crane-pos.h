#ifndef     BRAKE_CRANE_POS_H
#define     BRAKE_CRANE_POS_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct brake_crane_pos_t
{
    int pos_num;
    double axis_y_min;
    double axis_y_max;

    brake_crane_pos_t()
        : pos_num(1)
        , axis_y_min(0)
        , axis_y_max(0)
    {

    }
};

#endif
