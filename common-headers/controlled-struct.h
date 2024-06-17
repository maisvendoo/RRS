#ifndef     CLIENT_CONTROL_STRUCT_H
#define     CLIENT_CONTROL_STRUCT_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct controlled_t
{
    int     current_vehicle;
    int     controlled_vehicle;

    controlled_t()
        : current_vehicle(-1)
        , controlled_vehicle(-1)
    {

    }
};

#endif // CLIENT_CONTROL_STRUCT_H
