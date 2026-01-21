#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

class Route;

namespace vsg
{

class AmbientLight;

}

class SceneGraph : public vsg::Inherit<vsg::Group, SceneGraph>
{
public:
    SceneGraph();

    vsg::ref_ptr<Route> get_route() const;

private:
    vsg::ref_ptr<Route> route;
    vsg::ref_ptr<vsg::AmbientLight> ambient_light;
};

#endif // SCENE_GRAPH_H
