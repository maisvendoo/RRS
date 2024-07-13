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
    osg::Vec3   position;
    osg::Vec3   orth;
    osg::Vec3   up;
    osg::Vec3   right;
    osg::Vec3   attitude;
    int         orientation;
    osg::Vec3   driver_pos;

    animations_t *anims;
    displays_t   *displays;
    std::vector<size_t> sounds_id;

    vehicle_exterior_t()
        : transform(new osg::MatrixTransform())
        , cabine(nullptr)
        , position(osg::Vec3(0.0, 0.0, 0.0))
        , orth(osg::Vec3(0.0, 0.0, 0.0))
        , up(osg::Vec3(0.0, 0.0, 1.0))
        , attitude(osg::Vec3(0.0, 0.0, 0.0))
        , orientation(1)
        , driver_pos(osg::Vec3(0.0, 0.0, 0.0))
        , anims(new animations_t())
        , displays(new displays_t())
    {
        sounds_id.clear();
    }
};

#endif // VEHICLE_EXTERIOR_H
