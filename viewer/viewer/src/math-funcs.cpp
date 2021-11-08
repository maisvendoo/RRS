#include    "math-funcs.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float arg(float cos_x, float sin_x)
{
    float angle = 0;

    if (sin_x >= 0.0f)
        angle = acosf(cos_x);
    else
        angle = 2.0 * M_PI - acosf(cos_x);

    return angle;
}
