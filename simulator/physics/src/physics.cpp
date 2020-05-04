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

const double Physics::g = 9.81;
const double Physics::kmh = 3.6;
const double Physics::pA = 101325.0;
const double Physics::PI = 3.1415926;
const double Physics::MPa = 1e6;
const double Physics::Rmu = 8.31;
const double Physics::Mair = 0.029;
const double Physics::c = 340.0;
const double Physics::ZERO = 1e-10;
const double Physics::RAD = Physics::PI / 180.0;

const double Physics::FricApproxCoeff = 1e3;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Physics::sign(double x)
{
    if (x == 0)
        return 0;
    else
    {
        if (x > 0)
            return 1;
        else
            return -1;
    }
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
