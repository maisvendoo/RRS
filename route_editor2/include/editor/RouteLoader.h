#ifndef EDITOR_ROUTE_LOADER_H
#define EDITOR_ROUTE_LOADER_H

#include <string>
#include <thread>

class RouteLoader
{
public:
    std::thread load_static_objects_thread;
    std::thread load_topology_thread;

public:
    void start_load_route(const std::string& route_dir);

private:
    void load_static_objects(const std::string& route_dir);

    bool load_objects_ref(const std::string& route_dir);

    void load_route_map(const std::string& route_dir);

    void load_topology(const std::string& route_dir);
};

#endif // EDITOR_ROUTE_LOADER_H
