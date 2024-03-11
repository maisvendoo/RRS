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
    QString profile_path;
    double  prof_step;
    double  coeff_to_wheel_rail_friction;
    QString train_config;
    QString route_dir;
    int     integration_time_interval;
    int     control_time_interval;
    int     keys_buffer_size;
    bool    debug_print;
    solver_config_t solver_config;
    bool    is_long_forces_print;
    double  long_forces_time_step;

    init_data_t()
        : init_coord(0)
        , init_velocity(0)
        , direction(1)
        , profile_path("")
        , prof_step(100.0)
        , coeff_to_wheel_rail_friction(1.0)
        , train_config("")
        , route_dir("")
        , integration_time_interval(100)
        , control_time_interval(50)
        , keys_buffer_size(1024)
        , debug_print(false)
        , solver_config(solver_config_t())
        , is_long_forces_print(false)
        , long_forces_time_step(0.01)
    {

    }
};

#endif // INIT_DATA_H

