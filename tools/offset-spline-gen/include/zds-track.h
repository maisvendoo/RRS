#ifndef     ZDS_TRACK_H
#define     ZDS_TRACK_H

#include    <string>
#include    "vec3.h"

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
struct zds_track_t
{
    dvec3       begin_point = dvec3(0.0, 0.0, 0.0);
    dvec3       end_point = dvec3(0.0, 0.0, 0.0);
    int         prev_uid = -1;
    int         next_uid = -2;
    std::string arrows = "";
    int         voltage = 0;
    int         ordinate = 0;

    double      length = 0.0;
    double      route_coord = 0.0;
    double      railway_coord = 0.0;
    double      railway_coord_end = 0.0;
    double      inclination = 0.0;
    double      curvature = 0.0;
    dvec3       orth = dvec3(0.0, 1.0, 0.0);
    dvec3       right = dvec3(1.0, 0.0, 0.0);
    dvec3       up = dvec3(0.0, 0.0, 1.0);
    dvec3       trav = dvec3(0.0, 1.0, 0.0);
};

#endif
