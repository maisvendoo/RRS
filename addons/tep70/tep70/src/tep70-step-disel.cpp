#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepDisel(double t, double dt)
{
    disel->step(t, dt);
}