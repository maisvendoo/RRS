#ifndef     DEFAULT_COUPLING_H
#define     DEFAULT_COUPLING_H

#include    "coupling.h"

/*!
 *  \class
 *  \brief Default spring coupling
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SpringCoupling : public Coupling
{
public:

    SpringCoupling();
    ~SpringCoupling();

    double getForce(double ds, double dv);

private:

    /// Shiffness
    double c;

    void loadConfig(QString cfg_path);
};


#endif // DEFAULT_COUPLING_H
