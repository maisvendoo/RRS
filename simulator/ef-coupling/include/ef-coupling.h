#ifndef     EFCOUPLING_H
#define     EFCOUPLING_H

#include    "coupling.h"

class EfCoupling : public Coupling
{
public:

    EfCoupling();
    ~EfCoupling();

    double getForce(double ds, double dv);

private:

    double c;
    double f;
    double T0;

    void loadConfig(QString cfg_path);
};

#endif // EFCOUPLING_H
