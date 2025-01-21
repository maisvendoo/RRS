#include "RouteLoader.h"
#include <vsg/nodes/Group.h>

RouteLoader::RouteLoader()
    : routeDir("")
    , root(vsg::Group::create())
{
}

vsg::Group* RouteLoader::getRoot()
{
    if (root)
    {
        return root.release_nodelete();
    }
    else
    {
        return nullptr;
    }
}

RouteLoader* loadRouteLoader(const std::string& path, const std::string& name)
{
    // RouteLoader* loader = nullptr;
    return nullptr;
}

RouteLoader::~RouteLoader() = default;
