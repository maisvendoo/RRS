#ifndef     SIMULATOR_INFO_STRUCT_H
#define     SIMULATOR_INFO_STRUCT_H

#include    "global-const.h"

#include    <array>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_route_info_t
{
    int  route_dir_name_length;
    wchar_t route_dir_name[ROUTE_DIR_NAME_SIZE];

    simulator_route_info_t()
        : route_dir_name_length(ROUTE_DIR_NAME_SIZE)
        , route_dir_name(L"")
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_info_t
{
    int  vehicle_config_dir_length;
    wchar_t vehicle_config_dir[VEHICLE_CONFIG_DIR_NAME_SIZE];
    int  vehicle_config_file_length;
    wchar_t vehicle_config_file[VEHICLE_CONFIG_FILENAME_SIZE];

    simulator_vehicle_info_t()
        : vehicle_config_dir_length(VEHICLE_CONFIG_DIR_NAME_SIZE)
        , vehicle_config_dir(L"")
        , vehicle_config_file_length(VEHICLE_CONFIG_FILENAME_SIZE)
        , vehicle_config_file(L"")
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_info_t
{
    int num_updates;
    simulator_route_info_t route_info;
    int  num_vehicles;
    std::array<simulator_vehicle_info_t, MAX_NUM_VEHICLES>  vehicles_info;

    simulator_info_t()
        : num_updates(-1)
        , num_vehicles(0)
    {

    }
};

#endif // SIMULATOR_INFO_STRUCT_H
