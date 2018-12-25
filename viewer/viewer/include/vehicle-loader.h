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
 *
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Group *loadVehicle(const std::string &configPath);

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::MatrixTransform *loadWheels(const std::string &configPath);

#endif // VEHICLE_LOADER_H
