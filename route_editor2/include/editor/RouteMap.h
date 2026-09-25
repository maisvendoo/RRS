#ifndef EDITOR_ROUTE_MAP_H
#define EDITOR_ROUTE_MAP_H

#include <vsg/maths/vec3.h>

#include <map>
#include <string>
#include <vector>

/// Трансформация объекта из route1.map
struct RouteMapTransformation
{
    vsg::dvec3 translation;
    vsg::dvec3 rotation_deg;
};

using RouteMap = std::map<std::string, std::vector<RouteMapTransformation>>;

#endif // EDITOR_ROUTE_MAP_H
