#ifndef MOTION_PATH_H
#define MOTION_PATH_H

#include "basis.h"

#include <vsg/maths/vec3.h>

class MotionPath
{
public:
    MotionPath();

    virtual vsg::vec3 getPosition(float railway_coord) = 0;
    virtual vsg::vec3 getPosition(float railway_coord, vsg::vec3& attitude) = 0;
    virtual vsg::vec3 getPosition(float railway_coord, vsg::vec3& attitude, basis_t& basis) = 0;
    virtual vsg::vec3 getPosition(float railway_coord, basis_t& basis) = 0;

    float getLength() const;

protected:
    float length;
};

#endif // MOTION_PATH_H
