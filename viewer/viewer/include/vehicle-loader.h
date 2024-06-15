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
osg::Group *loadVehicle(const std::string &configDir, const std::string &configName);


/*!
 * \fn
 * \brief Load cabine module
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void loadCabine(osg::Group *vehicle,
                const std::string &configDir, const std::string &configName,
                osg::ref_ptr<osg::Node> &cabine_model);

/*!
 * \fn
 * \brief Get lehgth of vehicle
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float getLength(const std::string &configDir, const std::string &configName);

osg::Vec3 getDirverPosition(const std::string &configDir, const std::string &configName);

#endif // VEHICLE_LOADER_H
