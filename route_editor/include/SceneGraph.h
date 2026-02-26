#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include "EditorContext.h"
#include "SwitchGroup.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>

class Route;
struct settings_t;

namespace vsg
{

class AmbientLight;
class Options;
class Viewer;

}

class SceneGraph : public vsg::Inherit<SwitchGroup, SceneGraph>
{
public:
    SceneGraph(const EditorContext& context);

    void load_route();

    vsg::ref_ptr<Route> get_route() const;

private:
    const EditorContext& context;

    vsg::ref_ptr<Route> route;
    vsg::ref_ptr<vsg::AmbientLight> ambient_light;
};

#endif // SCENE_GRAPH_H
