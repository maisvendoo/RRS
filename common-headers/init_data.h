//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

#ifndef     INIT_DATA_H
#define     INIT_DATA_H

#include    <QString>

#include    "solver-config.h"

/*!
 * \struct
 * \brief
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct init_data_t
{
    double  init_coord;
    double  init_velocity;
    int     direction;
    QString route_dir_name;
    QString train_config;
    double  coeff_to_wheel_rail_friction;
    int     integration_time_interval;
    int     control_time_interval;
    bool    debug_print;
    solver_config_t solver_config;

    init_data_t()
        : init_coord(0.0)
        , init_velocity(0.0)
        , direction(1)
        , route_dir_name("")
        , train_config("")
        , coeff_to_wheel_rail_friction(1.0)
        , integration_time_interval(15)
        , control_time_interval(15)
        , debug_print(false)
    {
        solver_config = solver_config_t();
    }
};

#endif // INIT_DATA_H

