#ifndef ROUTE_H
#define ROUTE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/Switch.h>

#include <map>
#include <string>
#include <string_view>

struct EditorContext;

class Route : public vsg::Inherit<vsg::Switch, Route>
{
public:
    std::string route_dir;

public:
    Route(EditorContext& editor_ctx);

    void load();

    std::map<std::string_view, vsg::ref_ptr<vsg::PagedLOD>>& get_paged_lods()
    {
        return paged_lods;
    }

private:
    bool load_stations_conf();
    bool load_waypoints_conf();

    void load_static_objects();
    bool load_topology();

private:
    EditorContext& editor_ctx;
    std::map<std::string_view, vsg::ref_ptr<vsg::PagedLOD>> paged_lods;
};

#endif // ROUTE_H
