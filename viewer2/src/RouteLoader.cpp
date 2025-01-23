#include "RouteLoader.h"
#include "Library.h"
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
    RouteLoader* loader = nullptr;
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

RouteLoader::~RouteLoader() = default;
