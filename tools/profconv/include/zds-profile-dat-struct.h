#ifndef     ZDS_PROFILE_H
#define     ZDS_PROFILE_H

#include    <string>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_profile_element_t
{
    int         ord;
    float       uklon;
    short int   Radius;
    short int   LeftHeight;
    short int   RightHeight;
    short int   RodTok;
    short int   IsDoubleTrack;
    short int   LeftForestType;
    short int   LeftForestRemoval;
    short int   RightForestType;
    short int   RightForestRemoval;
    short int   LeftFonType;
    short int   LeftFonRemoval;
    short int   RightFonType;
    short int   RightFonRemoval;
    short int   LeftFenceType;
    short int   LeftFenceRemoval;
    short int   RightFenceType;
    short int   RightFenceRemoval;
    short int   LeftTracks;
    short int   RightTracks;
    short int   LeftOpors;
    short int   RightOpors;
    short int   LeftWires;
    short int   RightWires;
    std::string str_R;
    std::string str_L;
    short int   LeftFieldType;
    short int   LeftFieldAngle;
    short int   RightFieldType;
    short int   RightFieldAngle;
};

#endif // ZDS_PROFILE_H
