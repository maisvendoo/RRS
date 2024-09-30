#ifndef     CAMERA_POSITION_H
#define     CAMERA_POSITION_H

#include    <QMetaType>

#include    <osg/Vec3>
#include    <osg/Vec3d>

#include    "basis.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct camera_position_t
{
    osg::Vec3d   position;
    osg::Vec3d   attitude;
    osg::Vec3d   driver_pos;
    osg::Vec3d   viewer_pos;
    osg::Vec3d   front;
    osg::Vec3d   right;
    osg::Vec3d   up;
    bool        is_orient_bwd;

    camera_position_t()
        : position(osg::Vec3d())
        , attitude(osg::Vec3d())
        , driver_pos(osg::Vec3d())
        , viewer_pos(osg::Vec3d())
        , front(osg::Y_AXIS)
        , right(osg::X_AXIS)
        , up(osg::Z_AXIS)
    {

    }
};

Q_DECLARE_METATYPE(camera_position_t)

#endif // CAMERA_POSITION_H
