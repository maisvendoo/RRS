#ifndef     VEHICLE_EXTERIOR_H
#define     VEHICLE_EXTERIOR_H

#include    <osg/MatrixTransform>
#include    "animations-list.h"
#include    "display-container.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct vehicle_exterior_t
{
    osg::ref_ptr<osg::MatrixTransform> transform;
    osg::ref_ptr<osg::Node> cabine;
    osg::Vec3d  position;
    osg::Vec3d  orth;
    osg::Vec3d  up;
    osg::Vec3d  right;
    osg::Vec3d  attitude;
    int         orientation;
    osg::Vec3d  driver_pos;

    animations_t *anims;
    displays_t   *displays;
    std::vector<size_t> sounds_id;

    vehicle_exterior_t()
        : transform(new osg::MatrixTransform())
        , cabine(nullptr)
        , position(osg::Vec3d(0.0, 0.0, 0.0))
        , orth(osg::Vec3d(0.0f, 0.0f, 0.0f))
        , up(osg::Vec3d(0.0f, 0.0f, 1.0f))
        , attitude(osg::Vec3d(0.0f, 0.0f, 0.0f))
        , orientation(1)
        , driver_pos(osg::Vec3d(0.0f, 0.0f, 0.0f))
        , anims(new animations_t())
        , displays(new displays_t())
    {
        sounds_id.clear();
    }
};

#endif // VEHICLE_EXTERIOR_H
