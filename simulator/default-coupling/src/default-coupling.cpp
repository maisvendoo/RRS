#include    "default-coupling.h"

#include    "physics.h"
#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SpringCoupling::SpringCoupling() : Coupling()
  , c(1e7)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SpringCoupling::~SpringCoupling()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double SpringCoupling::getForce(double ds, double dv)
{
    (void) dv;

    //double force = 0;
    //double ads = abs(ds);

    /*if ( ads <= delta / 2)
        force = 0;

    if ( (ads > delta / 2) && (ads <= lambda) )
        force = c * (ads - delta / 2);

    if (ads > delta / 2 + lambda)
        force = ck * (ads - delta / 2 - lambda);*/

    return c * ds;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SpringCoupling::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Coupling";

        cfg.getDouble(secName, "c", c);
    }
}

GET_COUPLING(SpringCoupling)
