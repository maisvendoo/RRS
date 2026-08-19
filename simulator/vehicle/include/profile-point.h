#ifndef     PROFILE_ELEMENT_H
#define     PROFILE_ELEMENT_H

#include    "vec3.h"

struct profile_point_t
{
    double  railway_coord;
    double  inclination;
    double  curvature;
    dvec3   position;
    dvec3   orth;
    dvec3   right;
    dvec3   up;

    profile_point_t()
        : railway_coord(0.0)
        , inclination(0.0)
        , curvature(0.0)
        , position(dvec3(0.0, 0.0, 0.0))
        , orth(dvec3(0.0, 0.0, 0.0))
        , right(dvec3(0.0, 0.0, 0.0))
        , up(dvec3(0.0, 0.0, 1.0))
    {

    }
};

#endif // PROFILE_ELEMENT_H
