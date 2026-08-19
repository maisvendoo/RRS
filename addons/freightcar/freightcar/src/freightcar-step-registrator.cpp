#include    "freightcar.h"

#include    "registrator.h"
#include    "airdistributor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreightCar::stepRegistrator(const double& t, const double& dt)
{
    (void) t;
    (void) dt;

    registrator->print(air_dist->getDebugMsg(), t, dt);
}
