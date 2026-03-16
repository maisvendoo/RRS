#ifndef VIEWER_ROUTE_H
#define VIEWER_ROUTE_H

#include <vsg/maths/vec3.h>

#include <map>
#include <string>
#include <vector>

struct RouteObjectTransform
{
    vsg::vec3 translation;
    vsg::vec3 rotation_deg;
};

struct Route
{
    std::map<std::string, std::string> object_ref;
    std::map<std::string, std::vector<RouteObjectTransform>> route_map;
};

#endif // VIEWER_ROUTE_H
