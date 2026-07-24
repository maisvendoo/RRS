#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include "Route.h"
#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Switch.h>

struct EditorContext;
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
        const std::string& route_dir
    );

    void load_route();

private:
    EditorContext& context_;
    const camera_settings_t& camera_settings;
    const vsg::ref_ptr<vsg::Options>& vsg_options;
    vsg::ref_ptr<Route>& route;
    const std::string& route_dir;

    vsg::ref_ptr<vsg::AmbientLight> ambient_light_;
};

#endif // SCENE_GRAPH_H
