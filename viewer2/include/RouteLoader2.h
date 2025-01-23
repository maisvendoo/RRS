#ifndef ROUTE_LOADER2_H
#define ROUTE_LOADER2_H

#include <string>

struct Route;

class RouteLoader2
{
public:
    RouteLoader2(const std::string& route_path);

    void read_description();
    bool parse_objects_ref(Route& route);
    bool parse_route_map(Route& route);

private:
    std::string route_path;
    std::string route_type;
    std::string objects_ref_path;
    std::string route_map_path;
};

#endif // ROUTE_LOADER2_H
