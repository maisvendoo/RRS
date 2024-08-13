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

        // Светофор
        SPLIT_SIGNAL_FWD = 7,
        SPLIT_SIGNAL_BWD = 8,
    };

    size_t track_id = 0;

    std::vector<size_t> split_type = {};

    dvec3 point = {0.0, 0.0, 0.0};

    double railway_coord = 0.0;

    trajectory_t *fwd_main_traj = nullptr;

    trajectory_t *bwd_main_traj = nullptr;

    trajectory_t *fwd_side_traj = nullptr;

    trajectory_t *bwd_side_traj = nullptr;

    std::string signal_fwd_type = "";

    std::string signal_bwd_type = "";

    std::string signal_fwd_liter = "";

    std::string signal_bwd_liter = "";

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

#endif // TRAJECTORY_STRUCT_H
