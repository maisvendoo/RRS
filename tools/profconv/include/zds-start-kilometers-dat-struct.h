#ifndef     ZDS_START_KM_H
#define     ZDS_START_KM_H

#include    <string>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_start_km_t
{
    std::string name = "";
    size_t      forward_track_id = 0;
    size_t      backward_track_id = 0;

    double  forward_trajectory_coord = 0.0;
    double  forward_railway_coord = 0.0;
    double  backward_trajectory_coord = 0.0;
    double  backward_railway_coord = 0.0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_start_km_t>   zds_start_km_data_t;

#endif // ZDS_START_KM_H
