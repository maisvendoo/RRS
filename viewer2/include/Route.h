#ifndef VIEWER_ROUTE_H
#define VIEWER_ROUTE_H

#include <map>
#include <string>
#include <vector>

struct RouteObjectTransform
{
    float t_x;
    float t_y;
    float t_z;
    float r_x;
    float r_y;
    float r_z;
};

struct Route
{
    std::map<std::string, std::string> object_ref;
    std::multimap<std::string, RouteObjectTransform> transforms;
};

#endif // VIEWER_ROUTE_H
