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
    QString route_dir_name = "experimental-polygon";
    QString train_config = "vl60pk-1543";
    QString trajectory_name = "route1_0001";
    int     direction = 1;
    double  init_coord = 0.0;
    double  init_velocity = 0.0;
    double  coeff_to_wheel_rail_friction = 1.0;
    int     integration_time_interval = 15;
    int     control_time_interval = 15;
    bool    debug_print = false;
    solver_config_t solver_config = solver_config_t();
};

#endif // INIT_DATA_H
