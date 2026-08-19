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

#include <cstddef>

/*!
 * \struct
 * \brief Solver configuration data
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct solver_config_t final
{
    /// ODE solution method (solver name)
    QString     method = "euler";
    /// Intital time
    double      start_time = 0.0;
    /// Stop integration time
    double      stop_time = 10.0;
    /// Initial time step value (step value for fixed step methods)
    double      step = 3.0e-3;
    /// Maximal step value
    double      max_step = 3.0e-3;
    /// Number of substep
    std::size_t num_sub_step = 1;
    /// Local error of solution
    double      local_error = 1.0e-5;
};

#endif // SOLVER_CONFIG_H
