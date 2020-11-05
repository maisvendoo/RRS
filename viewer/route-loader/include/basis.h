#ifndef     BASIS_H
#define     BASIS_H

#include    <osg/Vec3>

struct basis_t
{
    osg::Vec3 front;
    osg::Vec3 right;
    osg::Vec3 up;

    basis_t()
        : front(osg::Y_AXIS)
        , right(osg::X_AXIS)
        , up(osg::Z_AXIS)
    {

    }
};

#endif // BASIS_H
