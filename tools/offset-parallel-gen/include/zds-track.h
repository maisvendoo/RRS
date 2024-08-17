#ifndef     ZDS_TRACK_H
#define     ZDS_TRACK_H

#include    <string>
#include    "vec3.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct point_t
{
    dvec3 point = {0.0, 0.0, 0.0};

    double railway_coord = 0.0;

    double trajectory_coord = 0.0;

    point_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_track_t
{
    dvec3       begin_point = dvec3(0.0, 0.0, 0.0);
    dvec3       end_point = dvec3(0.0, 0.0, 0.0);
    int         prev_uid = -1;
    int         next_uid = -2;
    std::string arrows = "";
    int         voltage = 0;
    int         ordinate = 0;

    double      length = 0.0;
    double      route_coord = 0.0;
    double      railway_coord = 0.0;
    double      railway_coord_end = 0.0;
    double      inclination = 0.0;
    double      curvature = 0.0;
    dvec3       orth = dvec3(0.0, 1.0, 0.0);
    dvec3       right = dvec3(1.0, 0.0, 0.0);
    dvec3       up = dvec3(0.0, 0.0, 1.0);
    dvec3       trav = dvec3(0.0, 1.0, 0.0);
};

#endif
