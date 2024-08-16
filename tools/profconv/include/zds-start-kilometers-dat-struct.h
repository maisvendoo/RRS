#ifndef     ZDS_START_KM_H
#define     ZDS_START_KM_H

#include    "trajectory_struct.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_start_km_t
{
    std::string name = "";
    size_t      forward_track_id = 0;
    size_t      backward_track_id = 0;

    double  forward_route_coord = 0.0;
    double  backward_route_coord = 0.0;

    start_point_data_t start_points = {};
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_start_km_t>   zds_start_km_data_t;

#endif // ZDS_START_KM_H
