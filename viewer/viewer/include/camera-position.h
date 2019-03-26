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
    basis_t     view_basis;

    camera_position_t()
        : position(osg::Vec3())
        , attitude(osg::Vec3())
        , driver_pos(osg::Vec3())
    {

    }
};

Q_DECLARE_METATYPE(camera_position_t)

#endif // CAMERA_POSITION_H
