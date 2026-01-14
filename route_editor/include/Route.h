#ifndef ROUTE_H
#define ROUTE_H

#include "PagedLodMap.h"
#include "RouteMap.h"
#include "StringMap.h"
#include "topology.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include <filesystem>
#include <string>
#include <vector>

class Signal;
struct settings_t;

namespace vsg
{

class Options;
class Viewer;

}

class Route : public vsg::Inherit<vsg::Group, Route>
{
public:
    std::filesystem::path directory;

public:
    bool load(
        const settings_t& settings,
        vsg::ref_ptr<vsg::Options> options,
        vsg::ref_ptr<vsg::Viewer> viewer
    );

    const StringMap& get_objects_ref() const;
    const RouteMap& get_route_map() const;
    const Topology& get_topology() const;

private:
    StringMap objects_ref;
    RouteMap route_map;
    Topology topology;

private:
    bool load_objects_ref();
    bool load_route_map();

    void load_static_objects(
        const PagedLodMap& paged_lods,
        vsg::ref_ptr<vsg::Viewer> viewer
    );

    bool load_topology(
        const settings_t& settings,
        vsg::ref_ptr<vsg::Options> options,
        vsg::ref_ptr<vsg::Viewer> viewer
    );

    void load_signals(
        const settings_t& settings,
        vsg::ref_ptr<vsg::Options> options,
        vsg::ref_ptr<vsg::Viewer> viewer,
        const std::vector<Signal*>& in_signals,
        const std::string& models_dir,
        PagedLodMap& paged_lods
    );
};

#endif // ROUTE_H
