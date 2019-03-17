#ifndef     VEHICLE_EXTERIOR_H
#define     VEHICLE_EXTERIOR_H

#include    <osg/MatrixTransform>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct vehicle_exterior_t
{
    osg::ref_ptr<osg::MatrixTransform> transform;
    osg::MatrixTransform *wheel_rotation;
    osg::ref_ptr<osg::Node> cabine;
    osg::Vec3   position;
    osg::Vec3   attitude;
    float       wheel_angle;
    float       coord;
    float       length;
    osg::Vec3   driver_pos;
};

#endif // VEHICLE_EXTERIOR_H
