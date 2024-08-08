#ifndef     TRACK_H
#define     TRACK_H

#include    <vec3.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct track_t
{
    dvec3 begin_point = {0.0, 0.0, 0.0};

    dvec3 end_point = {0.0, 0.0, 0.0};

    dvec3 orth = {0.0, 0.0, 0.0};

    dvec3 trav = {0.0, 0.0, 0.0};

    dvec3 up = {0.0, 0.0, 0.0};

    double len = 0.0;

    track_t()
    {

    }

    track_t(dvec3 p0, dvec3 p1)
    {
        begin_point = p0;
        end_point = p1;

        dvec3 t = end_point - begin_point;
        len = length(t);

        orth = normalize(t);

        trav.z = 0.0;
        trav.x = t.y;
        trav.y = t.x;

        trav = normalize(trav);

        up = cross(trav, t);
        up = normalize(up);
    }
};

#endif
