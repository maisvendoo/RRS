#ifndef     ZDS_SPEEDS_H
#define     ZDS_SPEEDS_H

#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_speeds_t
{
    size_t  begin_track_id = 0;
    size_t  end_track_id = 0;
    double  limit = 120.0;

    double  begin_trajectory_coord = 0.0;
    double  end_trajectory_coord = 0.0;

    static bool compare_by_track_id(const zds_speeds_t left, const zds_speeds_t right)
    {
        return left.begin_track_id < right.begin_track_id;
    };
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_speeds_t>   zds_speeds_data_t;

#endif // ZDS_SPEEDS_H
