#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepDisel(double t, double dt)
{
    disel->setQ_emn(electro_oil_pump->getOilFlow());
    disel->step(t, dt);
}
