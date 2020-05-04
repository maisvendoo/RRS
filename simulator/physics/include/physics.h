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
    static const double g;
    /// Normal atmosphere pressure
    static const double pA;
    /// Kilometers per hour coefficient
    static const double kmh;
    /// PI number
    static const double PI;
    /// Megapascales coefficient
    static const double MPa;
    /// Universal gas constant
    static const double Rmu;
    /// Molar mass of air
    static const double Mair;
    /// Sound speed
    static const double c;
    /// Zero equivalent
    static const double ZERO;
    /// Radian
    static const double RAD;

    /*!
     * \brief Signum function
     * \param x
     * \return
     */
    static double sign(double x);

    /*!
     * \fn
     * \brief Cut value by range
     */
    template <typename T>
    static T cut(T value, T min, T max)
    {
        if (value < min)
            return min;

        if (value > max)
            return max;

        return value;
    }

    /*!
     * \brief Calculate friction force as active force
     * \param Fmax - maximal friction force value
     * \param v - relative slip velocity
     * \return - friction force
     */
    static double fricForce(double Fmax, double v);

    static double gapForce(double x, double c, double lambda);

    static double gapMotion(double x, double a);

    /// Constructor
    Physics(){}
    /// Destructor
    ~Physics(){}

private:

    static const double FricApproxCoeff;
};

#endif // PHYSICS_H
