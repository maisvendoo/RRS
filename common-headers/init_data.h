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
    QString train_config;
    int     integration_time_interval;
    bool    debug_print;
    solver_config_t solver_config;

    init_data_t()
        : init_coord(0)
        , init_velocity(0)
        , direction(1)
        , profile_path("")
        , prof_step(100.0)
        , train_config("")
        , integration_time_interval(100)
        , debug_print(false)
    {

    }
};

#endif // INIT_DATA_H

