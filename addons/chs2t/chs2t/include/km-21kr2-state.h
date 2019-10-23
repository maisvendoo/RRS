#ifndef KM21KR2STATE_H
#define KM21KR2STATE_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct ControllerState
{
    bool up;
    bool up1;
    bool zero;
    bool down1;
    bool down;

    bool k01;
    bool k02;

    bool k31;
    bool k32;
    bool k33;

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
