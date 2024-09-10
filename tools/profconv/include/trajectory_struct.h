#ifndef TRAJECTORY_STRUCT_H
#define TRAJECTORY_STRUCT_H

#include    <vector>
#include    <string>
#include    <vec3.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct point_t
{
    dvec3 point = {0.0, 0.0, 0.0};

    double railway_coord = 0.0;

    double trajectory_coord = 0.0;

    point_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct trajectory_t
{
    std::string name = "";
    std::vector<point_t> points = {};

    trajectory_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<trajectory_t *> route_trajectories_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct split_zds_trajectory_t
{
    enum
    {
        // Съезды между главными путями
        SPLIT_2MINUS2 = 1,
        SPLIT_2PLUS2 = 2,

        // Съезды в начале/конце однопутного участка
        SPLIT_2_1 = 3,
        SPLIT_1_2 = 4,

        // Стрелки на боковые пути
        SPLIT_TO_SIDE = 5,
        SPLIT_FROM_SIDE = 6,

        // Стрелки на бок, прописанные в .trk, но без прописанных боковых путей
        SPLIT_TO_SIDE_NO_BRANCH = 7,
        SPLIT_FROM_SIDE_NO_BRANCH = 8,

        // Светофор
        SPLIT_SIGNAL_FWD = 9,
        SPLIT_SIGNAL_BWD = 10,

        // Начало нового пикетажа
        SPLIT_NEW_RAILWAY_COORD = 11
    };

    size_t track_id = 0;

    std::vector<size_t> split_type = {};

    dvec3 point = {0.0, 0.0, 0.0};

    double railway_coord = 0.0;

    std::string fwd_main_traj = "";

    std::string bwd_main_traj = "";

    std::string fwd_side_traj = "";

    std::string bwd_side_traj = "";

    std::string signal_fwd_type = "";

    std::string signal_bwd_type = "";

    std::string signal_fwd_liter = "";

    std::string signal_bwd_liter = "";

    double length_bwd_traj = 0.0;

    split_zds_trajectory_t()
    {

    }

    static bool compare_by_track_id(const split_zds_trajectory_t* left, const split_zds_trajectory_t* right)
    {
        return (*left).track_id < (*right).track_id;
    };
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<split_zds_trajectory_t *> route_connectors_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct start_point_t
{
    std::string name = "";
    std::string trajectory_name = "";
    int         direction = 1;
    double      trajectory_coord = 0.0;
    double      railway_coord = 0.0;

    double station_id = 0;

    start_point_t()
    {

    }

    static bool compare_by_direction(const start_point_t* left, const start_point_t* right)
    {
        return (*left).direction < (*right).direction;
    };
    static bool compare_by_station_id(const start_point_t* left, const start_point_t* right)
    {
        return (*left).station_id < (*right).station_id;
    };
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<start_point_t *>   start_point_data_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct speed_element_t
{
    int     limit = 0;
    int     railway_coord_begin = -1;
    int     railway_coord_end = 1000000000;

    speed_element_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct speedmap_t
{
    std::vector<std::string> trajectories_names = {};
    std::vector<speed_element_t> speedmap_elements = {};

    speedmap_t()
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<speedmap_t *>   speedmap_data_t;

#endif // TRAJECTORY_STRUCT_H
