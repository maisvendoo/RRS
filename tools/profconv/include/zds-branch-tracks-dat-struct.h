#ifndef     ZDS_BRANCH_TRACK_H
#define     ZDS_BRANCH_TRACK_H

#include    <string>
#include    <vector>

#include    "vec3.h"

/// Отклонение на соседний главный путь в ZDS задаётся смещением -7.47
const double ZDS_CONST_BIAS_FOR_OTHER_MAIN_TRACK = -7.47;

/// Моделируем траекторию отклонения пятью отрезками
/// с четырями промежуточными точками по кубическому сплайну (x*x*(3-2*x))
const size_t NUM_BIAS_POINTS = 4;
const double COORD_COEFF[NUM_BIAS_POINTS] = {0.2, 0.4, 0.6, 0.8};
const double BIAS_COEFF[NUM_BIAS_POINTS] =
{
    COORD_COEFF[0] * COORD_COEFF[0] * (3.0 - 2.0 * COORD_COEFF[0]),
    COORD_COEFF[1] * COORD_COEFF[1] * (3.0 - 2.0 * COORD_COEFF[1]),
    COORD_COEFF[2] * COORD_COEFF[2] * (3.0 - 2.0 * COORD_COEFF[2]),
    COORD_COEFF[3] * COORD_COEFF[3] * (3.0 - 2.0 * COORD_COEFF[3])
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

    int     branch_track_id = 0;
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
    std::vector<zds_branch_point_t> branch_points = {};
    bool    to_other_main_track = false;
    bool    from_other_main_track = false;

    std::vector<calculated_branch_point_t> branch_trajectory = {};
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<zds_branch_track_t *> zds_branch_track_data_t;

#endif // ZDS_BRANCH_TRACK_H
