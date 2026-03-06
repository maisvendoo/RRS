#ifndef ROUTE_MAP_H
#define ROUTE_MAP_H

#include <vsg/maths/vec3.h>

#include <map>
#include <string>
#include <vector>

struct RouteMapTransformation
{
    vsg::vec3 translation;
    vsg::vec3 rotation;
};

using RouteMap = std::map<std::string, std::vector<RouteMapTransformation>>;

#endif // ROUTE_MAP_H
