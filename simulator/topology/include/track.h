#ifndef     TRACK_H
#define     TRACK_H

#include    <vec3.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Track
{
public:

    Track() = default;

    ~Track() = default;

private:

    dvec3 begin_point;

    dvec3 end_point;

    dvec3 orth;

    dvec3 trav;

    dvec3 up;
};

#endif
