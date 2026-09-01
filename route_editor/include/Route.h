#ifndef ROUTE_H
#define ROUTE_H

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Switch.h>

#include <string>

struct EditorContext;
class Gizmo;
class ObjectManager;
struct camera_settings_t;

class Route : public vsg::Inherit<vsg::Switch, Route>
{
public:
    Route(
        EditorContext& context,
        const camera_settings_t& camera_settings,
        const vsg::ref_ptr<vsg::Options>& vsg_options,
        const std::string& route_dir,
        const vsg::ref_ptr<Gizmo>& gizmo,
        ObjectManager& object_manager
    );

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
    const vsg::ref_ptr<vsg::Options>& vsg_options;
    const std::string& route_dir;
    const vsg::ref_ptr<Gizmo>& gizmo;
    ObjectManager& object_manager;
};

#endif // ROUTE_H
