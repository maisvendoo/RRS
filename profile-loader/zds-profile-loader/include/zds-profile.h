//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     ZDS_PROFILE_H
#define     ZDS_PROFILE_H

#include    <QtGlobal>

struct zds_profile_element_t
{
    int     ord;
    float   uklon;
    qint16  Radius;
    qint16  LeftHeight;
    qint16  RightHeight;
    qint8   RodTok;
    qint8   IsDoubleTrack;
    qint16  LeftForestType;
    qint16  LeftForestRemoval;
    qint16  RightForestType;
    qint16  RightForestRemoval;
    qint8   LeftFonType;
    qint8   LeftFonRemoval;
    qint8   RightFonType;
    qint8   RightFonRemoval;
    qint8   LeftFenceType;
    qint8   LeftFenceRemoval;
    qint8   RightFenceType;
    qint8   RightFenceRemoval;
    qint8   LeftTracks;
    qint8   RightTracks;
    qint8   LeftOpors;
    qint8   RightOpors;
    qint8   LeftWires;
    qint8   RightWires;
    QString str_R;
    QString str_L;
    qint8   LeftFieldType;
    qint8   LeftFieldAngle;
    qint8   RightFieldType;
    qint8   RightFieldAngle;
};

#endif // ZDSPROFILE_H
