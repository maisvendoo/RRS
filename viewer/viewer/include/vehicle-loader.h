//------------------------------------------------------------------------------
//
//      Functions for loading external model of vehicle
//      (c) maisvendoo, 24/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Functions for loading external model of vehicle
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 24/12/2018
 */

#ifndef     VEHICLE_LOADER_H
#define     VEHICLE_LOADER_H

#include    <osg/MatrixTransform>

#include    "vehicle-exterior.h"

/*!
 * \fn
 * \brief Load vehicle body model
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Group *loadVehicle(const std::string &configPath);

/*!
 * \fn
 * \brief Load wheels model
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::MatrixTransform *loadWheels(const std::string &configPath);

void setAxis(osg::Group *vehicle,
             osg::MatrixTransform *wheel,
             const std::string &config_name);

#endif // VEHICLE_LOADER_H
