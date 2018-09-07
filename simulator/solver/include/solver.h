//------------------------------------------------------------------------------
//
//      Abstract solver of ODE system
//      (c) maisvendoo, 03/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Abstract solver of ODE system
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 03/09/2018
 */

#ifndef     SOLVER_H
#define     SOLVER_H

#include    "solver-types.h"
#include    "solver-export.h"
#include    "ode-system.h"

/*!
 * \class
 * \brief Abstract ODE solver
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SOLVER_EXPORT Solver
{
public:

    /// Constructor
    Solver() {}
    /// Destructor
    virtual ~Solver() {}

    /*!
     * \brief ODE system integration step
     * \param ode_sys - ODE system object
     * \param Y - state vector
     * \param dYdt - devivative of state vector by time
     * \param t - current time
     * \param dt - current timestep
     * \param max_step - maximal timestep
     * \param local_err - local error of solution
     * \return -
     */
    virtual bool step(OdeSystem *ode_sys,
                      state_vector_t &Y,
                      state_vector_t &dYdt,
                      double t,
                      double &dt,
                      double max_step,
                      double local_err) = 0;

    virtual void init(size_t ode_order) = 0;
};

/*!
 * \typedef
 * \brief GetSolver function signature
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef Solver* (*GetSolver)();

/*!
 * \def
 * \brief Marco for generate getSolver() function
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_SOLVER(ClassName) \
    extern "C" Q_DECL_EXPORT Solver *getSolver() \
    {\
        return new (ClassName)(); \
    }

/*!
 * \fn
 * \brief Load solver from library
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT Solver *loadSolver(QString lib_path);

#endif // SOLVER_H
