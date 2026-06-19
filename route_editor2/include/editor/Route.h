#ifndef EDITOR_ROUTE_H
#define EDITOR_ROUTE_H

#include "editor/RouteMapTransformation.h"

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class ObjectManager;

class Route
{
public:
    explicit Route(const std::unique_ptr<ObjectManager>& object_manager);

    std::thread load_static_objects_thread;
    std::thread load_topology_thread;

public:
    void start_load(const std::string& route_dir);

    void join_threads();

private:
    using Label = std::string;
    using RelativePath = std::string;

private:
    const std::unique_ptr<ObjectManager>& object_manager;

    std::map<Label, RelativePath> objects_ref;
    std::map<Label, std::vector<RouteMapTransformation>> route_map;

private:
    void load_static_objects(const std::string& route_dir);

    bool load_objects_ref(const std::string& route_dir);

    void print_objects_ref_in_journal() const;

    bool load_route_map(const std::string& route_dir);

    void print_route_map_in_journal() const;

    void load_topology(const std::string& route_dir);
};

#endif // EDITOR_ROUTE_H
