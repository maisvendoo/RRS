#ifndef KM21KR2STATE_H
#define KM21KR2STATE_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct ControllerState
{
    bool k01;
    bool k02;

    bool k21;
    bool k22;
    bool k23;
    bool k25;

    bool k31;
    bool k32;
    bool k33;

    ControllerState()
        : k01(false)
        , k02(false)
        , k21(false)
        , k22(false)
        , k23(false)
        , k25(false)
        , k31(false)
        , k32(false)
        , k33(false)
    {

    }
};

#endif // KM21KR2STATE_H
