#ifndef     ZDS_SIGNALS_AT_MAP_H
#define     ZDS_SIGNALS_AT_MAP_H

#include    <string>
#include    <vector>

#include    "vec3.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_signal_position_t
{
    std::string obj_name = "";
    dvec3       position = dvec3(0.0, 0.0, 0.0);
    dvec3       attitude = dvec3(0.0, 0.0, 0.0);

    std::string type = "";
    std::string liter = "";
    int         direction = 0;
    int         route_num = -1;
    int         track_id = -1;
    int         branch_num = -1;
    double      distance_from_main = 0.0;
    double      track_coord = 0.0;
    double      attitude_z_angle = 0.0;

    dvec3       nearest_point = dvec3(0.0, 0.0, 0.0);
    dvec3       rho_right = dvec3(0.0, 0.0, 0.0);
    dvec3       track_right = dvec3(0.0, 0.0, 0.0);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_signal_position_t *>    zds_signals_at_map_data_t;

#endif // ZDS_SIGNALS_AT_MAP_H
