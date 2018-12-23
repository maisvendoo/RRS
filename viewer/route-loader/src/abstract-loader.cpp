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

#include    "abstract-loader.h"

#include    "library.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteLoader::RouteLoader()
    : routeDir("")
    , root(new osg::Group)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Group *RouteLoader::getRoot()
{
    if (root.valid())
        return root.release();
    else
        return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Trajectory *RouteLoader::getTrajectory()
{
    return train_traj.get();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MotionPath *RouteLoader::getMotionPath()
{
    return train_traj->getMotionPath();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RouteLoader *loadRouteLoader(const std::string &path, const std::string &name)
{
    RouteLoader *loader = nullptr;

    Library lib(path, name);

    if (lib.load())
    {
        GetRouteLoader getRouteLoader = (GetRouteLoader) lib.resolve("getRouteLoader");

        if (getRouteLoader)
        {
            loader = getRouteLoader();
        }
    }

    return loader;
}
