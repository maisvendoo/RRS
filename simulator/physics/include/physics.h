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

#ifndef     PHYSICS_H
#define     PHYSICS_H

#include    <QtGlobal>
#include    "math-funcs.h"

#if defined(PHYSICS_LIB)
    #define PHYSICS_EXPORT  Q_DECL_EXPORT
#else
    #define PHYSICS_EXPORT  Q_DECL_IMPORT
#endif

using namespace std;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class PHYSICS_EXPORT Physics
{
public:

    /// Earth gravity acceleration
    static constexpr double g = 9.81;
    /// Normal atmosphere pressure
    static constexpr double pA = 101325.0;
    /// Kilometers per hour coefficient
    static constexpr double kmh = 3.6;
    /// PI number
    static constexpr double PI = 3.1415926;
    /// Megapascales coefficient
    static constexpr double MPa = 1.0e6;
    /// Universal gas constant
    static constexpr double Rmu = 8.31;
    /// Molar mass of air
    static constexpr double Mair = 0.029;
    /// Sound speed
    static constexpr double c = 340.0;
    /// Zero equivalent
    static constexpr double ZERO = 1.0e-10;

    /*!
     * \brief Signum function
     * \param x
     * \return
     */
    static double sign(double x);

    /*!
     * \brief Calculate friction force as active force
     * \param Fmax - maximal friction force value
     * \param v - relative slip velocity
     * \return - friction force
     */
    static double fricForce(double Fmax, double v);

    static double gapForce(double x, double c, double lambda);

    static double gapMotion(double x, double a);

private:
    static constexpr double FricApproxCoeff = 100.0;
};

#endif // PHYSICS_H
