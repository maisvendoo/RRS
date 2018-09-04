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
    QString train_config_path;
    int     integration_time_interval;

    init_data_t()
        : init_coord(0)
        , init_velocity(0)
        , direction(1)
        , profile_path("")
        , train_config_path("")
        , integration_time_interval(100)
    {

    }
};

#endif // INIT_DATA_H
