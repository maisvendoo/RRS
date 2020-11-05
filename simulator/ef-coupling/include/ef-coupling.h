//------------------------------------------------------------------------------
//
//      Elastic-friction train coupling model
//      (c) maisvendoo, 09/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Elastic-friction train coupling model
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 09/09/2018
 */

#ifndef     EFCOUPLING_H
#define     EFCOUPLING_H

#include    "coupling.h"

/*!
 * \class
 * \brief Elastic-friction train coupling
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
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
