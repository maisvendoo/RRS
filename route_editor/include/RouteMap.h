#ifndef ROUTE_MAP_H
#define ROUTE_MAP_H

#include <map>
#include <string>

#include <vsg/maths/vec3.h>

struct RouteMapTransformation
{
    vsg::vec3 translation;
    vsg::vec3 rotation;
};

using RouteMap = std::multimap<std::string, RouteMapTransformation>;

#endif // ROUTE_MAP_H
