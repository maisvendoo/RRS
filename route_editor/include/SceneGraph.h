#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include "SwitchGroup.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/core/ref_ptr.h>

struct EditorContext;
class Route;

namespace vsg
{

class AmbientLight;

}

class SceneGraph : public vsg::Inherit<SwitchGroup, SceneGraph>
{
public:
    SceneGraph(EditorContext& context);

    void load_route();

    vsg::ref_ptr<Route> get_route() const;

private:
    EditorContext& context;

    vsg::ref_ptr<Route> route;
    vsg::ref_ptr<vsg::AmbientLight> ambient_light;
};

#endif // SCENE_GRAPH_H
