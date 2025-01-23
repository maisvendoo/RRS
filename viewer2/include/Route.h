#ifndef VIEWER_ROUTE_H
#define VIEWER_ROUTE_H

#include <map>
#include <string>
#include <vector>

struct RouteObjectTransform
{
    float t_x;
    float t_y;
    float y_z;
    float r_x;
    float r_y;
    float r_z;
};

struct Route
{
    std::string type;
    std::vector<std::string> model_paths;
    std::multimap<std::string, RouteObjectTransform> transforms;
};

#endif // VIEWER_ROUTE_H
