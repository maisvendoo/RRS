#ifndef ROUTE_H
#define ROUTE_H

#include "PagedLodMap.h"
#include "RouteMap.h"
#include "StringMap.h"
#include "SwitchGroup.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>

#include <memory>

struct EditorContext;
class Topology;

class Route : public vsg::Inherit<SwitchGroup, Route>
{
public:
    Route(EditorContext& context);

    const StringMap& get_objects_ref() const;
    const RouteMap& get_route_map() const;
    const std::unique_ptr<Topology>& get_topology() const;

private:
    bool load_objects_ref();
    bool load_route_map();
    void load_static_objects(const PagedLodMap& paged_lods);

    bool load_topology();

private:
    EditorContext& context;

    StringMap objects_ref;
    RouteMap route_map;
    std::unique_ptr<Topology> topology;
};

#endif // ROUTE_H
