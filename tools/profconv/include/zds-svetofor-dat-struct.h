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
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_signals_t>   zds_signals_data_t;

#endif // ZDS_SVETOFOR_H
