#ifndef ROUTE_H
#define ROUTE_H

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Switch.h>

struct EditorContext;
struct camera_settings_t;

class Route : public vsg::Inherit<vsg::Switch, Route>
{
public:
    Route(EditorContext& context, const camera_settings_t& camera_settings);

private:
    bool load_objects_ref();
    bool load_route_map();
    bool load_stations_conf();
    bool load_waypoints_conf();

    void load_static_objects();
    bool load_topology();

private:
    EditorContext& context_;
    const camera_settings_t& camera_settings;
};

#endif // ROUTE_H
