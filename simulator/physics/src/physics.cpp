//------------------------------------------------------------------------------
//
//      Some mathematics functions and physical constants library
//      (c) maisvendoo, 03/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Some mathematics functions and physical constants library
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 03/09/2018
 */

#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Physics::sign(double x)
{
    return (x > 0.0) - (x < 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Physics::fricForce(double Fmax, double v)
{
    if (abs(v) < ZERO)
        return 0;

    return Fmax * tanh(FricApproxCoeff * v);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Physics::gapForce(double x, double c, double lambda)
{
    if (abs(x) <= lambda)
        return 0.0;
    else
    {
        if (x > lambda)
            return c * (x - lambda);

        if (x < -lambda)
            return c * (x + lambda);
    }

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Physics::gapMotion(double x, double a)
{
    if (abs(x) <= a)
        return 0.0;
    else
    {
        if (x > a)
            return x - a;

        if (x < -a)
            return x + a;
    }

    return 0.0;
}
