//------------------------------------------------------------------------------
//
//      Solver configuration data
//      (c) maisvendoo, 04/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Solver configuration data
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 04/09/2018
 */

#ifndef     SOLVER_CONFIG_H
#define     SOLVER_CONFIG_H

#include    <QString>

/*!
 * \struct
 * \brief Solver configuration data
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct solver_config_t
{
    /// ODE solution method (solver name)
    QString     method;
    /// Intital time
    double      start_time;
    /// Stop integration time
    double      stop_time;
    /// Initial time step value (step value for fixed step methods)
    double      step;
    /// Maximal step value
    double      max_step;
    /// Local error of solution
    double      local_error;

    solver_config_t()
        : method("rkf5")
        , start_time(0.0)
        , stop_time(10.0)
        , step(1e-3)
        , max_step(1e-2)
        , local_error(1e-5)
    {

    }
};

#endif // SOLVER_CONFIG_H
