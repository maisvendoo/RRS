#ifndef     ZDS_SVETOFOR_H
#define     ZDS_SVETOFOR_H

#include    <string>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_signals_t
{
    size_t      track_id = 0;
    std::string type = "";
    std::string liter = "";
    std::string special = "";

    double  trajectory_coord = 0.0;

    static bool compare_by_track_id(const zds_signals_t left, const zds_signals_t right)
    {
        return left.track_id < right.track_id;
    };
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_signals_t>   zds_signals_data_t;

#endif // ZDS_SVETOFOR_H
