#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vsg/core/Inherit.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Switch.h>

struct EditorContext;

namespace vsg
{

class AmbientLight;

}

class SceneGraph : public vsg::Inherit<vsg::Switch, SceneGraph>
{
public:
    SceneGraph(EditorContext& context);

    void load_route();

private:
    EditorContext& context_;

    vsg::ref_ptr<vsg::AmbientLight> ambient_light_;
};

#endif // SCENE_GRAPH_H
