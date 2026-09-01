#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Switch.h>

struct EditorContext;
class Gizmo;
class ObjectManager;
class Route;
struct camera_settings_t;

namespace vsg
{

class AmbientLight;

}

class SceneGraph : public vsg::Inherit<vsg::Switch, SceneGraph>
{
public:
    SceneGraph(
        EditorContext& context,
        const camera_settings_t& camera_settings,
        const vsg::ref_ptr<vsg::Options>& vsg_options,
        vsg::ref_ptr<Route>& route,
        const std::string& route_dir,
        const vsg::ref_ptr<Gizmo>& gizmo,
        ObjectManager& object_manager
    );

    void load_route();

private:
    EditorContext& context_;
    const camera_settings_t& camera_settings;
    const vsg::ref_ptr<vsg::Options>& vsg_options;
    vsg::ref_ptr<Route>& route;
    const std::string& route_dir;
    const vsg::ref_ptr<Gizmo>& gizmo;
    ObjectManager& object_manager;

    vsg::ref_ptr<vsg::AmbientLight> ambient_light_;
};

#endif // SCENE_GRAPH_H
