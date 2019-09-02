#ifndef KM21KR2STATE_H
#define KM21KR2STATE_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct ControllerState
{
    double up;
    double up1;
    double zero;
    double down1;
    double down;

    ControllerState()
        : up(0)
        , up1(0)
        , zero(0)
        , down1(0)
        , down(0)
    {

    }
};

#endif // KM21KR2STATE_H
