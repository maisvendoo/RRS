#ifndef     ZDS_MAP_H
#define     ZDS_MAP_H

#include    <string>
#include    <vector>

#include    "vec3.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_object_position_t
{
    std::string obj_name = "";
    dvec3       position = dvec3(0.0, 0.0, 0.0);
    dvec3       attitude = dvec3(0.0, 0.0, 0.0);
    std::string obj_info = "";
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_object_position_t *>    zds_route_map_data_t;

#endif // ZDS_MAP_H
