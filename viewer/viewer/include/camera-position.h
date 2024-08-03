#ifndef     CAMERA_POSITION_H
#define     CAMERA_POSITION_H

#include    <QMetaType>

#include    <osg/Vec3>

#include    "basis.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct camera_position_t
{
    osg::Vec3   position;
    osg::Vec3   attitude;
    osg::Vec3   driver_pos;
    osg::Vec3   viewer_pos;
    osg::Vec3   front;
    osg::Vec3   right;
    osg::Vec3   up;
    bool        is_orient_bwd;

    camera_position_t()
        : position(osg::Vec3())
        , attitude(osg::Vec3())
        , driver_pos(osg::Vec3())
        , viewer_pos(osg::Vec3())
        , front(osg::Y_AXIS)
        , right(osg::X_AXIS)
        , up(osg::Z_AXIS)
    {

    }
};

Q_DECLARE_METATYPE(camera_position_t)

#endif // CAMERA_POSITION_H
