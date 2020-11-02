#ifndef     PROFILE_ELEMENT_H
#define     PROFILE_ELEMENT_H

struct profile_element_t
{
    double  railway_coord;
    double  inclination;
    double  curvature;

    profile_element_t()
        : railway_coord(0.0)
        , inclination(0.0)
        , curvature(0.0)
    {

    }
};

#endif // PROFILE_ELEMENT_H
