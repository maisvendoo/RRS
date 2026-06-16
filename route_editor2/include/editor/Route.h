#ifndef EDITOR_ROUTE_H
#define EDITOR_ROUTE_H

#include <string>
#include <thread>

class Route
{
public:
    std::thread load_static_objects_thread;
    std::thread load_topology_thread;

public:
    void start_load(const std::string& route_dir);

private:
    void load_static_objects(const std::string& route_dir);

    bool load_objects_ref(const std::string& route_dir);

    void load_route_map(const std::string& route_dir);

    void load_topology(const std::string& route_dir);
};

#endif // EDITOR_ROUTE_H
