//------------------------------------------------------------------------------
//
//      Abstaract class for ODE system description
//      (c) maisvendoo, 03/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Abstaract class for ODE system description
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 03/09/2018
 */

#ifndef     ODE_SYSTEM_H
#define     ODE_SYSTEM_H

#include    <QObject>

#include    "solver-export.h"
#include    "solver-types.h"

/*!
 * \class
 * \brief Abstract ODE system
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SOLVER_EXPORT OdeSystem : public QObject
{
    Q_OBJECT

public:

    /// Constructor
    explicit OdeSystem(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~OdeSystem();
    /// Calculation of right part ODE system
    virtual void calcDerivative(state_vector_t &Y, state_vector_t &dYdt, double t) = 0;

protected:

    /// ODE state vector
    state_vector_t  y;
    /// State vector derivative by time
    state_vector_t  dydt;
};

#endif // ODE_SYSTEM_H
