//------------------------------------------------------------------------------
//
//      Abstract loader or railway route
//      (c) maisvendoo, 03/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Abstract loader or railway route
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 03/12/2018
 */

#ifndef		ABSTRACT_LOADER_H
#define		ABSTRACT_LOADER_H

#include    <osg/Referenced>
#include    <osg/Group>
#include    <osgGA/GUIEventHandler>

#include    "import-export.h"

#ifdef ROUTE_LOADER_LIB
    #define ROUTE_LOADER_EXPORT DECL_EXPORT
#else
    #define ROUTE_LOADER_EXPORT DECL_IMPORT
#endif

#include    "abstract-trajectory.h"
#include    "abstract-path.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum ReadResult
{
    FILE_READ_SUCCESS,
    FILE_NOT_FOUND,
    FILE_NOT_HANDLED
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ROUTE_LOADER_EXPORT RouteLoader : public osg::Referenced
{
public:

    /// Constructor
    RouteLoader();

    /// Load route
    virtual void load(std::string routeDir) = 0;

    /// Get route scene group node
    virtual osg::Group *getRoot();

    /// Get camera events handler
    virtual osgGA::GUIEventHandler *getCameraEventHandler(int direction,
                                                          float camera_height) = 0;

    Trajectory *getTrajectory();

    MotionPath *getMotionPath();

protected:    

    /// Route directory
    std::string routeDir;

    /// Route static scene root node
    osg::ref_ptr<osg::Group>    root;

    osg::ref_ptr<Trajectory>    train_traj;

    /// Destructor
    virtual ~RouteLoader() {}

    /// Load data from route config file
    virtual ReadResult loadDataFile(const std::string &filepath) = 0;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef RouteLoader* (*GetRouteLoader)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_ROUTE_LOADER(ClassName) \
    extern "C" DECL_EXPORT RouteLoader *getRouteLoader() \
    {\
        return new (ClassName)(); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DECL_EXPORT RouteLoader *loadRouteLoader(const std::string &path,
                                                    const std::string &name);

#endif
