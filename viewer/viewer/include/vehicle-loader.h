//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_LOADER_H
#define     VEHICLE_LOADER_H

#include    <osg/MatrixTransform>

#include    "vehicle-exterior.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Group *loadVehicle(const std::string &configPath);

osg::MatrixTransform *loadWheels(const std::string &configPath);

#endif // VEHICLE_LOADER_H
