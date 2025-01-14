#ifndef     CAMERA_POSITION_H
#define     CAMERA_POSITION_H

#include    <QMetaType>

#include    <osg/Vec3d>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct camera_position_t
{
    osg::Vec3d  position = {0.0, 0.0, 0.0};
    osg::Vec3d  attitude = {-osg::PI_2, 0.0, 0.0};
    osg::Vec3d  driver_pos = {0.0, 0.0, 1.75};
    osg::Vec3d  viewer_pos = {0.0, 150.0, 0.0};
    osg::Vec3d  front = {0.0, 1.0, 0.0};
    osg::Vec3d  right = {1.0, 0.0, 0.0};
    osg::Vec3d  up = {0.0, 0.0, 1.0};
    bool        is_orient_bwd = false;

    camera_position_t()
    {

    }
};

Q_DECLARE_METATYPE(camera_position_t)

#endif // CAMERA_POSITION_H
