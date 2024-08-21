#ifndef     ZDS_BRANCH_TRACK_H
#define     ZDS_BRANCH_TRACK_H

#include    <string>
#include    <vector>

#include    "vec3.h"
#include    "trajectory_struct.h"

/// Отклонение на соседний главный путь в ZDS задаётся смещением -7.47
const double ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK = -7.47;

/// Моделируем траекторию отклонения пятью отрезками
/// с четырями промежуточными точками по полиномиальному сплайну
/// пятой степени 6*x^5 - 15*x^4 + 10*x^3
/* с четырями промежуточными точками по кубическому сплайну -2*x^3 + 3*x^2 */
const size_t NUM_BIAS_POINTS = 4;
const double COORD_COEFF[NUM_BIAS_POINTS] = {0.2, 0.4, 0.6, 0.8};
const double BIAS_COEFF[NUM_BIAS_POINTS] =
{
    COORD_COEFF[0] * COORD_COEFF[0] * COORD_COEFF[0] * (6.0 * COORD_COEFF[0] * COORD_COEFF[0] - 15.0 * COORD_COEFF[0] + 10.0),
    COORD_COEFF[1] * COORD_COEFF[1] * COORD_COEFF[1] * (6.0 * COORD_COEFF[1] * COORD_COEFF[1] - 15.0 * COORD_COEFF[1] + 10.0),
    COORD_COEFF[2] * COORD_COEFF[2] * COORD_COEFF[2] * (6.0 * COORD_COEFF[2] * COORD_COEFF[2] - 15.0 * COORD_COEFF[2] + 10.0),
    COORD_COEFF[3] * COORD_COEFF[3] * COORD_COEFF[3] * (6.0 * COORD_COEFF[3] * COORD_COEFF[3] - 15.0 * COORD_COEFF[3] + 10.0)
/*
    COORD_COEFF[0] * COORD_COEFF[0] * (3.0 - 2.0 * COORD_COEFF[0]),
    COORD_COEFF[1] * COORD_COEFF[1] * (3.0 - 2.0 * COORD_COEFF[1]),
    COORD_COEFF[2] * COORD_COEFF[2] * (3.0 - 2.0 * COORD_COEFF[2]),
    COORD_COEFF[3] * COORD_COEFF[3] * (3.0 - 2.0 * COORD_COEFF[3])
*/
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_branch_point_t
{
    int     main_track_id = 0;
    double  bias = 0.0;
    std::string signal_liter = "";
    std::string signal_special = "";

    int     dir = 1;
    int     trajectory_point_id = -1;
    bool    is_signal = false;
    dvec3   nearest_signal_pos = dvec3(0.0, 0.0, 0.0);
    int     nearest_signal_main_track_id = -1;
    int     trajectory_point_id_with_signal = -1;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct calculated_branch_point_t
{
    dvec3       point = dvec3(0.0, 0.0, 0.0);
    double      trajectory_coord = 0.0;
    double      railway_coord = 0.0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_branch_track_t
{
    std::vector<zds_branch_point_t *> branch_points = {};

    int id_begin = -1;
    int id_end = -1;
    bool begin_at_other = false;
    bool end_at_other = false;
    int id_begin_at_other = -1;
    int id_end_at_other = -1;

    std::vector<calculated_branch_point_t> branch_trajectory = {};

    route_trajectories_t trajectories = {};

    static bool compare_by_track_id_begin(const zds_branch_track_t* left, const zds_branch_track_t* right)
    {
        return left->id_begin < right->id_begin;
    };

    static bool compare_by_track_id_end(const zds_branch_track_t* left, const zds_branch_track_t* right)
    {
        return left->id_end < right->id_end;
    };
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_branch_track_t *> zds_branch_track_data_t;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_branch_2_2_t
{
    int id1 = -1;
    int id2 = -1;
    bool is_fwd = false;
    zds_branch_point_t  branch_point_fwd = zds_branch_point_t();
    bool is_bwd = false;
    zds_branch_point_t  branch_point_bwd = zds_branch_point_t();
    trajectory_t trajectory = {};

    static bool compare_by_track_id1(const zds_branch_2_2_t* left, const zds_branch_2_2_t* right)
    {
        return left->id1 < right->id1;
    };

    static bool compare_by_track_id2(const zds_branch_2_2_t* left, const zds_branch_2_2_t* right)
    {
        return left->id2 < right->id2;
    };
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_branch_2_2_t *> zds_branch_2_2_data_t;

#endif // ZDS_BRANCH_TRACK_H
