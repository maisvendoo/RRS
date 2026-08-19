#ifndef EDITOR_ROUTE_H
#define EDITOR_ROUTE_H

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Switch.h>

struct EditorContext;

/// Загруженный маршрут: статические объекты + топология
class Route : public vsg::Inherit<vsg::Switch, Route>
{
public:
    explicit Route(EditorContext& context);

private:
    bool load_objects_ref();
    bool load_route_map();
    bool load_stations_conf();
    bool load_waypoints_conf();

    void load_static_objects();
    bool load_topology();

private:
    EditorContext& context_;
};

#endif // EDITOR_ROUTE_H
