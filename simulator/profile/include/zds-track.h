#ifndef     ZDS_TRACK_H
#define     ZDS_TRACK_H

#include    <string>
#include    <vector>
#include    "vec3.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_track_t
{
    dvec3       begin_point;
    dvec3       end_point;
    int         prev_uid;
    int         next_uid;
    std::string arrows;
    int         voltage;
    int         ordinate;

    double      length;
    double      railway_coord;
    double      inclination;
    double      curvature;
    dvec3       orth;
    dvec3       right;
    dvec3       up;

    zds_track_t()
        : begin_point(dvec3(0.0, 0.0, 0.0))
        , end_point(dvec3(0.0, 0.0, 0.0))
        , prev_uid(-1)
        , next_uid(-2)
        , arrows("")
        , voltage(0)
        , ordinate(0)
        , length(0.0)
        , railway_coord(0.0)
        , inclination(0.0)
        , curvature(0.0)
        , orth(dvec3(0.0, 0.0, 0.0))
        , right(dvec3(0.0, 0.0, 0.0))
        , up(dvec3(0.0, 0.0, 1.0))
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_track_t> zds_track_data_t;

#endif // ZDS_TRACK_H
