#include "SceneGraph.h"

#include "EditorContext.h"
#include "Mask.h"
#include "Route.h"

#include <vsg/core/Mask.h>
#include <vsg/lighting/AmbientLight.h>

SceneGraph::SceneGraph(EditorContext& context)
    : context(context)
{
    ambient_light = vsg::AmbientLight::create();
    this->addChild(vsg::Mask{MASK_SCENE}, ambient_light);
}

void SceneGraph::load_route()
{
    context.route = Route::create(context);

    context.compile_infos.emplace_back(CompileInfo{
        context.scene_graph, context.route, vsg::MASK_ALL});
}
