#include    "ef-coupling.h"

#include    "physics.h"
#include    "CfgReader.h"

EfCoupling::EfCoupling()
{

}

EfCoupling::~EfCoupling()
{

}

double EfCoupling::getForce(double ds, double dv)
{
    double tmp = Physics::gapForce(ds, ck, delta / 2);
    double preForce = Physics::cut(tmp, -T0, T0);
    double gapForce = Physics::gapForce(ds, ck, delta / 2 + lambda);

    double x = Physics::gapMotion(ds, delta / 2);
    double force = c * x + Physics::fricForce(f * abs(c * x), dv);


    return preForce + gapForce + force;
}

void EfCoupling::loadConfig(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Coupling";
        cfg.getDouble(secName, "c", c);
        cfg.getDouble(secName, "f", f);
        cfg.getDouble(secName, "T0", T0);
    }
}

GET_COUPLING(EfCoupling)
