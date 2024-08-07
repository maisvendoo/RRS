#ifndef     ZDS_BRANCH_TRACK_H
#define     ZDS_BRANCH_TRACK_H

#include    <string>
#include    <vector>

#include    "vec3.h"

const double ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK = -7.47;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_branch_point_t
{
    int     main_track_id = 0;
    double  bias = 0.0;
    std::string signal_liter = "";
    std::string signal_special = "";

    static bool compare_by_track_id(const zds_branch_point_t left, const zds_branch_point_t right)
    {
        return left.main_track_id < right.main_track_id;
    };
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct calculated_branch_track_t
{
    dvec3       begin_point = dvec3(0.0, 0.0, 0.0);
    dvec3       end_point = dvec3(0.0, 0.0, 0.0);
    double      length = 0.0;
    double      trajectory_coord = 0.0;
    double      railway_coord = 0.0;
    double      railway_coord_end = 0.0;
    double      inclination = 0.0;
    double      curvature = 0.0;
    dvec3       orth = dvec3(0.0, 0.0, 0.0);
    dvec3       right = dvec3(0.0, 0.0, 0.0);
    dvec3       up = dvec3(0.0, 0.0, 0.0);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_branch_track_t
{
    std::vector<zds_branch_point_t> branch_track = {};
    bool    is_other_main_track = false;

    std::vector<calculated_branch_track_t> branch_trajectory = {};
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_branch_track_t> zds_branch_track_data_t;

#endif // ZDS_BRANCH_TRACK_H
