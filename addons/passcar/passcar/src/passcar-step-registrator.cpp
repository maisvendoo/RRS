#include    "passcar.h"

#include    "registrator.h"
#include    "airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PassCar::stepRegistrator(const double& t, const double& dt)
{
    (void) t;
    (void) dt;

    registrator->print(air_dist->getDebugMsg(), t, dt);
}
