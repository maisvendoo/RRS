#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vsg/core/ref_ptr.h>

class Route;

namespace vsg
{

class AmbientLight;
class Group;

}

class SceneGraph
{
private:
    vsg::ref_ptr<Route> route;
    vsg::ref_ptr<vsg::AmbientLight> ambient_light;
    vsg::ref_ptr<vsg::Group> gui_group;
};

#endif // SCENE_GRAPH_H
