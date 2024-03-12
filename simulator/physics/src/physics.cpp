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

const double Physics::FricApproxCoeff = 100.0;

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
double Physics::fricForce(double Fmax, double v, double beta)
{
    if (abs(v) < ZERO)
        return 0;

    return Fmax * tanh(beta * v);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Physics::gapForce(double x, double c, double lambda)
{
     return std::max(0.0, c * (x - lambda)) + std::min(0.0, c * (x + lambda));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Physics::gapMotion(double x, double a)
{
    return std::max(0.0, x - a) + std::min(0.0, x + a);
}
